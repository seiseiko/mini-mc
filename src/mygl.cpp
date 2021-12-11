#include "mygl.h"
#include <la.h>
#include <scene/noise.h>
#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(mkU<Cube>(this)), mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_camera(mkU<Camera>()), mp_terrain(mkU<Terrain>()),mp_texture_all(mkU<Texture>(this)),
      m_time(0.f), mp_progShadow(mkU<ShaderProgram>(this)),

      m_depthBuffer(-1), m_depthTexture(-1),
      // l-system river
      river_linear(mp_terrain.get(),this, glm::vec3(0, 128, 20)), river_delta(mp_terrain.get(),this, glm::vec3(0,128,50))
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);
    mp_terrain->t_context = this;
    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    // create 2 river expansions
    river_linear.main_wid = true;
    river_linear.river_wid = 3;
    river_linear.expansion["F"] = "FF";
    river_linear.expansion["F1"] = "-F+F";
    river_linear.expansion["F2"] = "F[+F--F]F[-F++F]F";
    river_linear.expansion["F3"] = "F";
    river_delta.river_wid = 3;
    river_delta.expansion["F"] = "FF+[+F-F-F]-[-F+F+F]";
    river_delta.expansion["F1"] = "++F--F";
    river_delta.expansion["F2"] = "--F++F";
    river_delta.expansion["F3"] = "[-F++F]F";
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_geomCube->destroy();
    for(int i = 0; i < mp_terrain->chunk_list.size(); i++)
    {
        mp_terrain->chunk_list[i]->destroy();
    }

}


void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    mp_geomCube->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    mp_progShadow->create(":/glsl/shadow.vert.glsl", ":/glsl/shadow.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.

    // Shadow Mapping related
    createShadowText();
    createTextureAll();



    mp_camera->RotateAboutUp(-8.5);
    mp_camera->RotateAboutRight(2.546);
    mp_camera->RecomputeAttributes();



    glBindVertexArray(vao);

    CreateTestScene();

    // create the river

    river_delta.axiom = "+F-F";
    river_delta.lsystemParse(3);

    createCave(150,150,70,150);

}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *mp_camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // shadow mapping： calculate a light point of view transformation
    // Compute the MVP matrix from the light's point of view
    glm::mat4 depthMVP;
    calculate_depthMVP(depthMVP);

    // Set necessary Matrices
    mp_progShadow->setDepthMVP(depthMVP);
    mp_progShadow->setViewProjMatrix(mp_camera->getViewProj());

    mp_progFlat->setDepthMVP(depthMVP);
    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progFlat->setCamPos(mp_camera->eye);
    mp_progFlat->setTime(m_time);

    mp_progLambert->setDepthMVP(depthMVP);
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setCamPos(mp_camera->eye);
    mp_progLambert ->setTime(m_time);

    m_time++;

    // render out the scene
    GLDrawScene();
}

void MyGL::GLDrawScene()
{

    // bind our depth frame buffer
    // one pass: shadow texture rendering
    bind_shadow_buffer();
    for(int i = 0; i < this->mp_terrain->chunk_list.size(); i++)
    {
        Chunk* c = this->mp_terrain->chunk_list[i].get();
        if(created.find(c) != created.end()){
            c->create_update();
        }else{
            c->create();
            created.insert(c);
        }
        mp_progShadow->setModelMatrix(glm::translate(glm::mat4(),c->pos));
        mp_progShadow->draw(*c,0,false);
    }
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    // bind default frame buffer back
    mp_texture_all->bind(2);
    bind_buffer();
    // two pass: normal rendering
    for(int i = 0; i < this->mp_terrain->chunk_list.size(); i++)
    {
        Chunk* c = this->mp_terrain->chunk_list[i].get();
        if(created.find(c) != created.end()){
            c->create_update();
        }else{
            c->create();
            created.insert(c);
        }
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(),c->pos));
        mp_progLambert->draw(*c,0,false);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(int i = 0; i < this->mp_terrain->chunk_list.size(); i++)
    {
        Chunk* c = this->mp_terrain->chunk_list[i].get();

        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(),c->pos));
        c->create_update();
        mp_progLambert->draw(*c,0,true);
    }
    glDisable(GL_BLEND);
}


void MyGL::keyPressEvent(QKeyEvent *e)
{

    float amount = 2.56781f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        mp_camera->RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        mp_camera->RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        mp_camera->RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        mp_camera->RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        mp_camera->fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        mp_camera->fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        mp_camera->TranslateAlongLook(amount);
        updateTerrainSize();
    } else if (e->key() == Qt::Key_S) {
        mp_camera->TranslateAlongLook(-amount);
        updateTerrainSize();
    } else if (e->key() == Qt::Key_D) {
        mp_camera->TranslateAlongRight(amount);
        updateTerrainSize();
    } else if (e->key() == Qt::Key_A) {
        mp_camera->TranslateAlongRight(-amount);
        updateTerrainSize();
    } else if (e->key() == Qt::Key_Q) {
        mp_camera->TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        mp_camera->TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        *mp_camera = Camera(this->width(), this->height());
    } else if (e->key() == Qt::Key_Z) {
        addBlock(BEDROCK);
    }else if (e->key() == Qt::Key_X) {
        addBlock(WOOD);
    }else if (e->key() == Qt::Key_C) {
        addBlock(LEAF);
    }else if (e->key() == Qt::Key_V) {
        addBlock(ICE);
    }else if (e->key() == Qt::Key_B) {
        addBlock(LAVA);
    }
    mp_camera->RecomputeAttributes();


}

void MyGL::mousePressEvent(QMouseEvent *m){
    if (m->button() == Qt::LeftButton){
        removeBlock();
    } else if (m ->button() == Qt::RightButton){
        addBlock(STONE);
   }
}

void MyGL::CreateTestScene()
{
    // Create the basic terrain floor

    for(int x = 0 ; x < CHUNK_SIZE * 6; ++x)
    {
        for(int z = - CHUNK_SIZE; z < CHUNK_SIZE * 4; ++z)
        {

            for(int y = 0; y < 256; ++y){
               mp_terrain->setBlockAt(x,y,z,EMPTY, this);
            }

        }
    }

    for(int x = 0 ; x < CHUNK_SIZE * 6; ++x)
    {
        for(int z = - CHUNK_SIZE * 2; z < CHUNK_SIZE * 4; ++z)
        {
            setBlockColumn(x,z);
        }
    }


    mp_terrain->update_adjacent();
    mp_terrain -> minX = 0;
    mp_terrain -> maxX = CHUNK_SIZE * 6;
    mp_terrain -> minZ =  - CHUNK_SIZE * 2;
    mp_terrain -> maxZ = CHUNK_SIZE * 4;


    //mp_terrain->create_update();
}

void MyGL::removeBlock(){

    glm::vec3 look = mp_camera -> look;
    glm::vec3 eye = mp_camera -> eye;
    float t = 1.f;



    while(t < 200.f){

        glm::vec3 marching_pos = eye + t * look;


        BlockType curr = mp_terrain -> getBlockAt(floor(marching_pos.x),floor(marching_pos.y),floor(marching_pos.z));

        if(curr != EMPTY){

            mp_terrain->setBlockAt(floor(marching_pos.x),floor(marching_pos.y),floor(marching_pos.z),EMPTY,this);

            break;

        }

        t = t + 0.1f;


    }


    mp_terrain->update_adjacent();
    mp_terrain->create_update();
}

void MyGL::addBlock(BlockType bt){
    glm::vec3 look = mp_camera -> look;

    glm::vec3 eye = mp_camera -> eye;

    float t = 1.f;


    while(t < 200.f){

        glm::vec3 marching_pos = eye + t * look;

       // std::cout<<t<<std::endl;


        BlockType curr = mp_terrain -> getBlockAt(floor(marching_pos.x),floor(marching_pos.y),floor(marching_pos.z));

        if(curr != EMPTY){

            glm::vec3 middle = glm::vec3(int(marching_pos.x) + 0.5f,int(marching_pos.y) + 0.5f,int(marching_pos.z)+ 0.5f);
            glm::vec3 delta = glm::normalize(marching_pos - middle);
            int index = 0;
            for(int i = 1; i < 3; i++){
                if(delta[index] <  delta[i]){
                    index = i;
                }
            }

            glm::vec3 normal(0);
            normal[index] = (look[index] > 0? -1 : 1);
            //std::cout<<normal[0]<<normal[1]<<normal[2]<<std::endl;

            glm::vec3 adjust(0);
            adjust[0] = (look[0] > 0? -1 : 1);
            adjust[1] = (look[1] > 0? -1 : 1);
            adjust[2] = (look[2] > 0? -1 : 1);


            glm::vec3 add_pos = glm::vec3(floor(marching_pos.x),floor(marching_pos.y),floor(marching_pos.z)) + normal;
           if(mp_terrain -> getBlockAt(add_pos.x,add_pos.y,add_pos.z) == EMPTY){
                mp_terrain->setBlockAt(add_pos.x,add_pos.y,add_pos.z,bt,this);
                //std::cout<<"added!!"<<std::endl;

            }else if(mp_terrain -> getBlockAt(add_pos.x,add_pos.y+1,add_pos.z) == EMPTY){
               mp_terrain->setBlockAt(add_pos.x,add_pos.y+1,add_pos.z,bt,this);

           }else if(mp_terrain -> getBlockAt(add_pos.x+adjust[0],add_pos.y+1,add_pos.z) == EMPTY){
               mp_terrain->setBlockAt(add_pos.x+adjust[0],add_pos.y+1,add_pos.z,bt,this);
           }else if(mp_terrain -> getBlockAt(add_pos.x,add_pos.y+1,add_pos.z+adjust[2]) == EMPTY){
               mp_terrain->setBlockAt(add_pos.x,add_pos.y+1,add_pos.z+adjust[2],bt,this);

           }else if(mp_terrain -> getBlockAt(add_pos.x+adjust[0],add_pos.y+1,add_pos.z+adjust[2])== EMPTY){
               mp_terrain->setBlockAt(add_pos.x+adjust[0],add_pos.y+1,add_pos.z+adjust[2],bt,this);
           }
            break;




        }

        t = t + 0.3;


    }


     mp_terrain->update_adjacent();
     mp_terrain->create_update();

}


void MyGL::updateTerrainSize(){
    glm::vec3 cam_pos = mp_camera -> eye;
    if(cam_pos.x > mp_terrain->maxX){

        for(int x = mp_terrain->maxX; x < mp_terrain->maxX + TERRAIN_SIZE; x++){
            for(int z = mp_terrain->minZ; z < mp_terrain ->maxZ; z++){
                setBlockColumn(x,z);
            }
        }
       mp_terrain->maxX =  mp_terrain->maxX + TERRAIN_SIZE;

       mp_terrain->create_update();

    }else if(cam_pos.x < mp_terrain->minX){


        for(int x = mp_terrain->minX - TERRAIN_SIZE; x < mp_terrain->minX; x++){
            for(int z = mp_terrain->minZ; z < mp_terrain ->maxZ; z++){
                setBlockColumn(x,z);
            }
        }
        mp_terrain->minX =  mp_terrain->minX - TERRAIN_SIZE;

        mp_terrain->create_update();
    }else if(cam_pos.z > mp_terrain->maxZ){


        for(int x = mp_terrain->minX; x < mp_terrain->maxX; x++){
            for(int z = mp_terrain->maxZ; z < mp_terrain ->maxZ + TERRAIN_SIZE; z++){
                setBlockColumn(x,z);
            }
        }
        mp_terrain->maxZ =  mp_terrain->maxZ + TERRAIN_SIZE;

        mp_terrain->create_update();
    }else if(cam_pos.z < mp_terrain->minZ){


        for(int x = mp_terrain->minX; x < mp_terrain->maxX; x++){
            for(int z = mp_terrain->minZ - TERRAIN_SIZE; z < mp_terrain ->minZ; z++){
                setBlockColumn(x,z);
            }
        }

        mp_terrain->create_update();
        mp_terrain->minZ =  mp_terrain->minZ - TERRAIN_SIZE;
    }

}

void MyGL::setBlockColumn(int x, int z){
    for(int y = 0; y <= 128; ++y){
       mp_terrain->setBlockAt(x,y,z,STONE, this);
    }

    float multiplier = fabs(fbm(abs(x),abs(z),0.5f,0.09f,3));

    multiplier = std::pow(multiplier,3.0);


    int upper_bound = (int)floor((255.f - 129.f) * multiplier + 129.f);
    if(upper_bound > max_bound){
        max_bound = upper_bound;
    }

    //std::cout<<max_bound<<std::endl;
    for(int y = 129; y <= upper_bound; ++y)
    {
        BlockType t = getBiomeBlockType(x,y,z,upper_bound);
        mp_terrain->setBlockAt(x,y,z,t, this);
    }



}

void MyGL::createTextureAll(){
    uPtr<Texture> texture_all = mkU<Texture>(this);
    texture_all -> create(":/textures/minecraft_textures_all.png");
    mp_texture_all = std::move(texture_all);
    mp_texture_all->load(2);

}



BlockType MyGL::getBiomeBlockType(int x, int y, int z, int upperBound){

        float moisture = fbm(x,z,3.f,0.2f,4);
        if(moisture > max){
            max = moisture;
        }
        if(moisture < min){
            min = moisture;
        }
        //std::cout<<moisture<<std::endl;



        if(y < 135 + int(3.f * noise2D(x,z))){
            if(moisture < 5.f){
                 return SAND;
            }else{
                if(y == upperBound){
                    return GRASS;
                }else{
                    return DIRT;
                }
            }
        }else if(y >= (135 + int(3.f * noise2D(x,z))) && y < (140 + int(3.f * noise2D(x,z)))){
            return STONE;
        }else{
            return SNOW;
        }
//        if(moisture < 5.f){
//            if(y < 135){
//                return SAND;
//            }else{
//                if(y == upperBound){
//                    return SNOW;
//                }else{
//                    return DIRT;
//                }
//            }
//        }else{
//            if(y == upperBound){
//                return GRASS;
//            }else{
//                return DIRT;
//            }
//        }

}




void MyGL::createCave(int steps,int start_x, int start_y,int start_z){

    //create tunnel
    glm::vec3 curr_pos(start_x,start_y,start_z);
    glm::vec3 pre_pos;

    glm::vec3 face_dir(1,0,0);
    int curr_step = steps;

    while(curr_step > 0){

        //create hollows
        for(int i = curr_pos[0] - 4; i<=curr_pos[0] + 4; i++){
              for(int j = curr_pos[1] - 4; j<=curr_pos[1] +4; j++){
                  for(int k = curr_pos[2] - 4; k<=curr_pos[2] + 4; k++){
                        glm::vec3 add_pos((float)i,(float)j,(float)k);
                        glm::vec3 delta = add_pos-curr_pos;

                        if(glm::length(delta) < 4.f){
                                mp_terrain->setBlockAt(i,j,k,EMPTY,this);

                        }else if(glm::length(delta) > 4.f &&glm::length(delta) < 4.5f ){
                            if((float)rand() / (float)RAND_MAX > 0.5f){
                                if(mp_terrain->getBlockAt(i,j,k) != EMPTY){
                                     mp_terrain->setBlockAt(i,j,k,GOLD,this);
                                }
                             }
                        }
                  }
              }
        }


        if(curr_step != steps){
            face_dir = curr_pos - pre_pos;
        }

        glm::vec3 marching_dir(fbm(curr_pos[0],curr_pos[1],3.f,2.5f,3), fbm(curr_pos[0],curr_pos[2],3.f,2.5f,3), fbm(curr_pos[1],curr_pos[2],3.f,2.5f,3));

        glm::vec3 normalized_marching_dir = glm::normalize(marching_dir);

        float dot_product = glm::dot(normalized_marching_dir,face_dir);

        if(dot_product < 0.f){
            normalized_marching_dir[0] = -normalized_marching_dir[0];
            normalized_marching_dir[1] = -normalized_marching_dir[1];
            normalized_marching_dir[2] = -normalized_marching_dir[2];

        }

        pre_pos = curr_pos;
        curr_pos = curr_pos + 5.f * normalized_marching_dir;

        curr_step--;
    }


    //The big cave
    glm::vec3 delta2 = curr_pos - pre_pos;
    curr_pos = pre_pos + 4.f *delta2;

    int min_y = 256;
    int min_x = 256;
    int min_z = 256;
    int max_x = 0;
    int max_z = 0;


    for(int i = curr_pos[0] - 16; i<=curr_pos[0] + 16; i++){
          for(int j = curr_pos[1] - 16;j<=curr_pos[1] +16; j++){
              for(int k = curr_pos[2] - 16; k<=curr_pos[2] + 16; k++){
                    glm::vec3 add_pos((float)i,(float)j,(float)k);
                    glm::vec3 delta = add_pos-curr_pos;

                    if(glm::length(delta) < (16.f + 3.f *noise3D(i,j,k))){
                            mp_terrain->setBlockAt(i,j,k,EMPTY,this);

                            if(i < min_x){
                               min_x = i;
                            }

                            if(i > max_x){
                                max_x = i;
                            }

                            if(j < min_y){
                                min_y = j;
                            }

                            if(k < min_z){
                                min_z = k;
                            }

                            if(k > max_z){
                                max_z = k;
                            }

                    }else if(glm::length(delta) > (16.f) &&glm::length(delta) < (18.f)  ){
                        if((float)rand() / (float)RAND_MAX > 0.5f){
                            if(mp_terrain->getBlockAt(i,j,k) != EMPTY){
                                 mp_terrain->setBlockAt(i,j,k,GOLD,this);
                            }
                         }
                    }
              }
          }
    }


    for(int i = min_x ; i <= max_x; i++){
        for(int j = min_y; j <= min_y +2; j++){
            for(int k = min_z; k <= max_z;k++){
                if(mp_terrain->getBlockAt(i,j,k) == EMPTY){
                        glm::vec3 add_pos((float)i,(float)j,(float)k);
                        glm::vec3 delta = add_pos-curr_pos;
                        if(glm::length(delta)<(16.f + 3.f *noise3D(i,j,k))){
                               mp_terrain->setBlockAt(i,j,k,LAVA,this);
                        }
                }
            }
        }
    }

}

