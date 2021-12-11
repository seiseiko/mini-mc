#include<mygl.h>
#include<QFileDialog>
#include<iostream>
// mm3: height map load
void MyGL::heightMap()
{

    // get the start position of our height map
    glm::vec3 look = mp_camera -> look;
    glm::vec3 eye = mp_camera -> eye;
    float t = 1.f;
    glm::vec3 start_pos;

    while(t < 200.f){
        start_pos = eye + t * look;
        BlockType curr = mp_terrain -> getBlockAt(floor(start_pos.x),floor(start_pos.y),floor(start_pos.z));
        if(curr != EMPTY){
            break;
        }
        t = t + 0.3;
    }
    start_pos = glm::vec3(floor(start_pos.x),floor(start_pos.y),floor(start_pos.z));
    // read file
    QString filename = QFileDialog::getOpenFileName(this,QString("Load Height Map"),
                                                    QDir::currentPath().append(QString("../..")),QString("*.PNG"));
    QImage height_image(filename);

    // iterate each pixel
    for (int ii = 0; ii < height_image.height(); ii++) {
        uchar* scan = height_image.scanLine(ii);
        int depth =4;
        for (int jj = 0; jj < height_image.width(); jj++) {
            QRgb* color = reinterpret_cast<QRgb*>(scan + jj*depth);

            glm::vec3 pos(start_pos[0] + ii, start_pos[1], start_pos[2] + jj);
            deform(color, pos);
        }
    }

    mp_terrain->create_update();
}

// deform the geometry
void MyGL::deform(QRgb* color, glm::vec3 pos)
{
    // get the grey scale
    int gray = qGray(*color);
    gray = glm::clamp(gray, 30, 200);
    int deform_value = gray / 5.f;
    // decide current x,z corrdinate type
    float r = qRed(*color), g = qGreen(*color), b = qBlue(*color);
    glm::vec3 c(r/256.f, g/256.f, g/256.f);
    BlockType t = rgb2type(c);

    // deform the original terrain
    int y = 160;
    while(y--){
        BlockType curr = mp_terrain -> getBlockAt(pos[0], y, pos[2]);
        if(curr != EMPTY){
            mp_terrain -> setBlockAt(pos[0], y + deform_value, pos[2], t, this);
        }
    }


}


BlockType MyGL::rgb2type(glm::vec3 c)
{

    glm::vec3 red(1, 0, 0);
    glm::vec3 green(0, 1, 0);
    glm::vec3 blue(0, 0, 1);
    glm::vec3 white(0, 0, 0);
    glm::vec3 black(1, 1, 1);

    float l[5];
    l[0] = glm::distance(c, red);
    l[1] = glm::distance(c, green);
    l[2] = glm::distance(c, blue);
    l[3] = glm::distance(c, white);
    l[4] = glm::distance(c, black);

    float index = 0;
    float min = l[0];
    for(int i = 0; i<5;i++){
        if(min >= l[i]){
            index = i;
            min = l[i];}
    }
    BlockType t;

    if(index==0){t = LAVA;}
    else if(index==1){t = GRASS;}
    else if(index==2){t = WATER;}
    else if(index==3){t = SNOW;}
    else if(index==4){t = DIRT;}
    return t;
}


