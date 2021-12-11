#include <scene/terrain.h>

#include <scene/cube.h>
#include<iostream>
#define EPSILON 0.00001

Terrain::Terrain() : dimensions(64, 256, 64)
{

}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    int start_x = int(floor(x / 16.f)) * 16;
    int start_z = int(floor(z / 16.f)) * 16;
    long long int key = (long long) start_x << 32 | (start_z  & 0x00000000FFFFFFFF);
    auto iter = mapping.find(key);
    if (iter != mapping.end())
    {
        Chunk* c = iter->second;
        glm::vec3 pos_local = glm::vec3(x, y, z) - c->pos;
        BlockType t = c->get_type2(int(pos_local[0]), int(pos_local[1]), int(pos_local[2]));
        return t;
    }
    else
    {
        return EMPTY;
    }
}

BlockType& Terrain::getBlockAt2(int x, int y, int z)
{
    // TODO: Make this work with your new block storage!
    int start_x = int(floor(x / 16.f)) * 16;
    int start_z = int(floor(z / 16.f)) * 16;
    long long int key = (long long) start_x << 32 | (start_z  & 0x00000000FFFFFFFF);
    auto iter = mapping.find(key);
    if (iter != mapping.end())
    {
        Chunk* c = iter->second;
        glm::vec3 pos_local = glm::vec3(x, y, z) - c->pos;
        BlockType& t = c->get_type2(int(pos_local[0]), int(pos_local[1]), int(pos_local[2]));
        return t;
    }
    else
    {
        return this->e;
    }

}


void Terrain::setBlockAt(int x, int y, int z, BlockType t,
                OpenGLContext* context)
{
    // TODO: Make this work with your new block storage!
    int start_x = int(floor(x / 16.f)) * 16;
    int start_z = int(floor(z / 16.f)) * 16;
    long long int key = (long long) start_x << 32 | (start_z  & 0x00000000FFFFFFFF);

    auto iter = mapping.find(key);
    if (iter != mapping.end())
    {
        Chunk* c = iter->second;
        glm::vec3 pos_local = glm::vec3(x, y, z) - c->pos;
        BlockType& tp = c->get_type2(int(pos_local[0]), int(pos_local[1]), int(pos_local[2]));
        tp = t;
    }
    else
    {
        uPtr<Chunk> c = mkU<Chunk>(context);
        c->pos = glm::vec3(start_x, 0, start_z);
        glm::vec3 pos_local = glm::vec3(x, y, z) - c->pos;
        BlockType& tp = c->get_type2(int(pos_local[0]), int(pos_local[1]), int(pos_local[2]));
        tp = t;

        this->chunk_list.push_back(std::move(c));

        Chunk* cp = chunk_list[chunk_list.size() - 1].get();
        mapping.insert(std::pair<long long int, Chunk*>(key,cp));

        // update the adjacent chunk
        // check front, back(z +- 16), left, right (x +-16 )

        long long int key_left = (long long) (start_x - 16) << 32 | start_z;
        long long int key_right = (long long) (start_x + 16) << 32 | start_z;
        long long int key_front = (long long) start_x << 32 | (start_z - 16);
        long long int key_back = (long long) start_x << 32 | (start_z + 16);

        iter = mapping.find(key_left);
        if(iter != mapping.end()){
            cp->left = iter->second;
        }

        iter = mapping.find(key_right);
        if(iter != mapping.end()){
            cp->right = iter->second;
        }

        iter = mapping.find(key_front);
        if(iter != mapping.end()){
            cp->front = iter->second;
        }

        iter = mapping.find(key_back);
        if(iter != mapping.end()){
            cp->back = iter->second;
        }


    }

}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 127; y < 256; ++y)
            {
                if(y <= 128)
                {
                    if((x + z) % 2 == 0)
                    {
                        m_blocks[x][y][z] = STONE;
                    }
                    else
                    {
                        m_blocks[x][y][z] = DIRT;
                    }
                }
                else
                {
                    m_blocks[x][y][z] = EMPTY;
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        m_blocks[x][129][0] = GRASS;
        m_blocks[x][130][0] = GRASS;
        m_blocks[x][129][63] = GRASS;
        m_blocks[0][130][x] = GRASS;
    }
    for(int y = 129; y < 140; ++y)
    {
        m_blocks[32][y][32] = GRASS;
    }

}

void Terrain::update_adjacent()
{
    for(int i = 0; i < chunk_list.size(); i++)
    {
         Chunk* cp = this->chunk_list[i].get();
         int start_x = int(cp->pos[0]);
         int start_z = int(cp->pos[2]);
         long long int key = (long long) start_x << 32 | (start_z  & 0x00000000FFFFFFFF);

         // check front, back(z +- 16), left, right (x +-16 )

         long long int key_left = (long long) (start_x - 16) << 32 | (start_z  & 0x00000000FFFFFFFF);
         long long int key_right = (long long) (start_x + 16) << 32 | (start_z  & 0x00000000FFFFFFFF);
         long long int key_front = (long long) start_x << 32 | ((start_z - 16) & 0x00000000FFFFFFFF) ;
         long long int key_back = (long long) start_x << 32 | ((start_z + 16)& 0x00000000FFFFFFFF);

         auto iter = mapping.find(key_left);
         if( (cp->left == nullptr) && (iter != mapping.end()) ){
             cp->left = iter->second;
         }

         iter = mapping.find(key_right);
         if( (cp->right == nullptr) && (iter != mapping.end()) ){
             cp->right = iter->second;
         }

         iter = mapping.find(key_front);
         if( (cp->front == nullptr) && (iter != mapping.end()) ){
             cp->front = iter->second;
         }

         iter = mapping.find(key_back);
         if( (cp->back == nullptr) && (iter != mapping.end()) ){
             cp->back = iter->second;
         }

    }
}

void Terrain::create_update()
{
    for(int i = 0; i < chunk_list.size(); i++)
    {
         Chunk* cp = this->chunk_list[i].get();
         cp->create();
    }
}

Chunk::Chunk(OpenGLContext *context):
    Drawable(context),
    front(nullptr),back(nullptr),left(nullptr),right(nullptr),
    pos(0, 0, 0)
{
    for(int i  = 0; i < 65535; i++)
    {
        type[i] = EMPTY;
    }
    front = nullptr;
    back = nullptr;
    left = nullptr;
    right = nullptr;
    color_list[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_list[STONE] = glm::vec4(0.5f);
    color_list[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_list[WATER] = glm::vec4(0, 0, 1, 1);
}
Chunk::~Chunk()
{

}

void  Chunk::destroy()
{

}

BlockType Chunk::get_type(int x, int y, int z) const
{
    int index = x + WIDTH * (y + HEIGHT * z);
    return type[index];
}

BlockType& Chunk::get_type2(int x, int y, int z)
{
    int index = x + WIDTH * (y + HEIGHT * z);
    return type[index];
}


void Chunk::create()
{

    std::vector<glm::vec4> vert;
    std::vector<GLuint> idx;
    std::vector<GLuint> idx_trans;
    std::vector<glm::vec2> uvs;
    std::vector<float> powers;
    int facenum;

    for(int i = 0; i < WIDTH; i++)
    {
        for(int j = 0; j < HEIGHT; j++)
        {
            for(int k = 0; k < DEPTH; k++)
            {
                BlockType t = get_type(i, j, k);
                glm::vec4 col = color_list[t];
                if( t != EMPTY )
                {
                    glm::vec4 pos1(i, j, k, 1),
                              pos2(i+1, j, k, 1),
                              pos3(i+1, j+1, k, 1),
                              pos4(i, j+1, k, 1),
                              pos5(i, j, k+1, 1),
                              pos6(i+1, j, k+1, 1),
                              pos7(i+1, j+1, k+1, 1),
                              pos8(i, j+1, k+1, 1);

                    // check adjacent block (six faces per block)

                    // front, vert : 1, 2, 3, 4

                    BlockType t_front = EMPTY;
                    if( k == 0 && (front != nullptr)){
                        t_front = front->get_type(i, j, 15);

                    }
                    else if(k > 0)
                    {
                        t_front = get_type(i, j, k-1);
                    }

                    if(t == WATER || t == ICE){
                        if(t_front == EMPTY){
                            push_face_data(pos1, pos2, pos3, pos4, vert, uvs,idx_trans, col,t,FRONT);
                            push_power(powers,t);
                        }
                    }else{
                        if( t_front == EMPTY||t_front == WATER || t_front == ICE ){
                            push_face_data(pos1, pos2, pos3, pos4, vert, uvs,idx, col,t,FRONT);
                            push_power(powers,t);

                        }
                    }




                    // back, vert: 8, 7, 6, 5

                    BlockType t_back = EMPTY;
                    if( k == 15 && (back != nullptr)) {
                        t_back = back->get_type(i, j, 0);
                    }
                    else if(k < 15)
                    {
                        t_back = get_type(i, j, k+1);
                    }

                    if(t == WATER || t == ICE){
                        if(t_back == EMPTY){
                            push_face_data(pos6, pos5, pos8, pos7, vert,uvs, idx_trans,col,t,BACK);
                            push_power(powers,t);

                        }
                    }else{
                        if(t_back == EMPTY || t_back == WATER || t_back == ICE){
                            push_face_data(pos6, pos5, pos8, pos7, vert,uvs, idx,col,t,BACK);
                            push_power(powers,t);

                        }
                    }


                    // up, vert: 4, 3, 7, 8

                    BlockType t_up = EMPTY;
                    if( j <  255 ){
                        t_up = get_type(i, j+1, k);
                    }


                    if(t == WATER || t == ICE){
                        if( t_up == EMPTY ){
                            push_face_data(pos4, pos3, pos7, pos8, vert,uvs,idx_trans, col,t,UP);
                            push_power(powers,t);


                        }
                    }else{
                        if(t_up == EMPTY || t_up == WATER || t_up == ICE){
                            push_face_data(pos4, pos3, pos7, pos8, vert,uvs,idx, col,t,UP);
                            push_power(powers,t);

                        }
                    }



                    // down, vert: 5, 6, 2, 1
                    BlockType t_down = EMPTY;
                    if( j > 0 ){
                        t_down = get_type(i, j-1, k);
                    }

                    if(t == WATER|| t == ICE){
                        if(t_down == EMPTY){
                            push_face_data(pos5, pos6, pos2, pos1, vert,uvs,idx_trans, col,t,DOWN);
                            push_power(powers,t);

                        }
                    }else{
                        if( t_down == EMPTY || t_down == WATER || t_down == ICE){
                            push_face_data(pos5, pos6, pos2, pos1, vert,uvs,idx, col,t,DOWN);
                            push_power(powers,t);

                        }
                    }



                    // left, vert: 5, 1, 4, 8
                    BlockType t_left = EMPTY;
                    if( i == 0 && (left != nullptr)){
                        t_left = left->get_type(15, j, k);
                    }
                    else if(i > 0)
                    {
                        t_left = get_type(i - 1, j, k);
                    }

                    if(t == WATER || t == ICE){
                        if(t_left == EMPTY){
                            push_face_data(pos5, pos1, pos4, pos8, vert,uvs,idx_trans, col,t,LEFT);
                            push_power(powers,t);

                        }
                    }else{
                        if(t_left == EMPTY || t_left == WATER || t_left == ICE ){
                            push_face_data(pos5, pos1, pos4, pos8, vert,uvs,idx, col,t,LEFT);
                            push_power(powers,t);

                        }
                    }



                    // right: vert: 2, 6, 7, 3
                    BlockType t_right = EMPTY;

                    if(i == 15 && (right != nullptr)){
                        t_right = right->get_type(0, j, k);
                    }
                    else if( i < 15 )
                    {
                        t_right = get_type(i + 1, j, k);
                    }

                    if(t == WATER|| t == ICE){
                        if(t_right == EMPTY){
                            push_face_data(pos2, pos6, pos7, pos3, vert,uvs,idx_trans, col,t,RIGHT);
                            push_power(powers,t);

                        }
                    }else{
                        if(t_right == EMPTY || t_right == WATER || t_right == ICE ){
                            push_face_data(pos2, pos6, pos7, pos3, vert,uvs,idx, col,t,RIGHT);
                            push_power(powers,t);

                        }
                    }

                }
            }

        }
    }

//    for(int i = 0; i < vert.size() / 12; i++)
//    {
//        idx.push_back(i*4);
//        idx.push_back(i*4+1);
//        idx.push_back(i*4+2);
//        idx.push_back(i*4);
//        idx.push_back(i*4+2);
//        idx.push_back(i*4+3);
//    }

    count = idx.size(); // Set "count" to the number of indices in your index VBO
    countTrans = idx_trans.size();

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generateIdxTrans();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_trans.size() * sizeof(GLuint), idx_trans.data(), GL_STATIC_DRAW);

    generateVert();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVert);
    context->glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(glm::vec4), vert.data(), GL_STATIC_DRAW);

    generateUV();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    context->glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

    generatePow();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPow);
    context->glBufferData(GL_ARRAY_BUFFER, powers.size() * sizeof(float), powers.data(), GL_STATIC_DRAW);



}
void Chunk::create_update(){
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVert);
    //context->glBindBuffer(GL_ARRAY_BUFFER,bufUV);
}


void Chunk::push_face_data(glm::vec4 pos1, glm::vec4 pos2, glm::vec4 pos3,
                           glm::vec4 pos4, std::vector<glm::vec4>& vert,std::vector<glm::vec2>& uvs,std::vector<GLuint>& idx,
                           glm::vec4 col,BlockType t,BlockSide side)
{

      // helper for create per face data
      glm::vec3 n = glm::normalize(glm::cross(glm::vec3(pos3 - pos2),glm::vec3(pos2 - pos1)));//glm::normalize(glm::cross(glm::vec3(pos2 - pos1), glm::vec3(pos3 - pos2)));
      glm::vec4 nor = glm::vec4(n, 1.f);

//      if(side == BACK){
//          std::cout<<n[0]<<n[1]<<n[2]<<std::endl;
//      }

      int start_idx = vert.size() / 3;
      idx.push_back(start_idx);
      idx.push_back(start_idx + 1);
      idx.push_back(start_idx + 2);
      idx.push_back(start_idx);
      idx.push_back(start_idx + 2);
      idx.push_back(start_idx + 3);



      vert.push_back(pos1);
      vert.push_back(nor);
      vert.push_back(col);

      vert.push_back(pos2);
      vert.push_back(nor);
      vert.push_back(col);

      vert.push_back(pos3);
      vert.push_back(nor);
      vert.push_back(col);

      vert.push_back(pos4);
      vert.push_back(nor);
      vert.push_back(col);

      glm::vec2 uv1;
      glm::vec2 uv2;
      glm::vec2 uv3;
      glm::vec2 uv4;

      //0 == front, 1 == back, 2 == up, 3 == down, 4 == left, 5 == right
      if(t == GRASS){
          if(side == FRONT || side == BACK || side == LEFT || side == RIGHT){
              uv1= glm::vec2(3.0/16.0,15.0/16.0);
              uv2= glm::vec2(4.0/16.0,15.0/16.0);
              uv3= glm::vec2(4.0/16.0,1.0);
              uv4= glm::vec2(3.0/16.0,1.0);
          }else if(side == UP){
              uv1= glm::vec2(8.0/16.0,13.0/16.0);
              uv2= glm::vec2(9.0/16.0,13.0/16.0);
              uv3= glm::vec2(9.0/16.0,14.0/16.0);
              uv4= glm::vec2(8.0/16.0,14.0/16.0);
          }else{
              uv1= glm::vec2(2.0/16.0,15.0/16.0);
              uv2= glm::vec2(3.0/16.0,15.0/16.0);
              uv3= glm::vec2(3.0/16.0,16.0/16.0);
              uv4= glm::vec2(2.0/16.0,16.0/16.0);
          }
      }else if(t == WATER){
          uv1= glm::vec2(13.0/16.0,3.0/16.0);
          uv2= glm::vec2(14.0/16.0,3.0/16.0);
          uv3= glm::vec2(14.0/16.0,4.0/16.0);
          uv4= glm::vec2(13.0/16.0,4.0/16.0);
      }else if(t == STONE){
          uv1= glm::vec2(1.0/16.0,15.0/16.0);
          uv2= glm::vec2(2.0/16.0,15.0/16.0);
          uv3= glm::vec2(2.0/16.0,16.0/16.0);
          uv4= glm::vec2(1.0/16.0,16.0/16.0);
      }else if(t == BEDROCK){
          uv1= glm::vec2(1.0/16.0,14.0/16.0);
          uv2= glm::vec2(2.0/16.0,14.0/16.0);
          uv3= glm::vec2(2.0/16.0,15.0/16.0);
          uv4= glm::vec2(1.0/16.0,15.0/16.0);
      }else if(t == WOOD){
          if(side == FRONT || side == BACK || side == LEFT || side == RIGHT){
              uv1= glm::vec2(4.0/16.0,14.0/16.0);
              uv2= glm::vec2(5.0/16.0,14.0/16.0);
              uv3= glm::vec2(5.0/16.0,15.0/16.0);
              uv4= glm::vec2(4.0/16.0,15.0/16.0);
          }else{
              uv1= glm::vec2(5.0/16.0,14.0/16.0);
              uv2= glm::vec2(6.0/16.0,14.0/16.0);
              uv3= glm::vec2(6.0/16.0,15.0/16.0);
              uv4= glm::vec2(5.0/16.0,15.0/16.0);
          }
      }else if(t == LEAF){
          uv1= glm::vec2(5.0/16.0,12.0/16.0);
          uv2= glm::vec2(6.0/16.0,12.0/16.0);
          uv3= glm::vec2(6.0/16.0,13.0/16.0);
          uv4= glm::vec2(5.0/16.0,13.0/16.0);
      }else if(t == ICE){
          uv1= glm::vec2(3.0/16.0,11.0/16.0);
          uv2= glm::vec2(4.0/16.0,11.0/16.0);
          uv3= glm::vec2(4.0/16.0,12.0/16.0);
          uv4= glm::vec2(3.0/16.0,12.0/16.0);
      }else if(t == LAVA){
          uv1= glm::vec2(13.0/16.0,1.0/16.0);
          uv2= glm::vec2(14.0/16.0,1.0/16.0);
          uv3= glm::vec2(14.0/16.0,2.0/16.0);
          uv4= glm::vec2(13.0/16.0,2.0/16.0);
      }else if(t == SNOW){
          uv1= glm::vec2(2.0/16.0,11.0/16.0);
          uv2= glm::vec2(3.0/16.0,11.0/16.0);
          uv3= glm::vec2(3.0/16.0,12.0/16.0);
          uv4= glm::vec2(2.0/16.0,12.0/16.0);
      }else if(t == SAND){
          uv1= glm::vec2(2.0/16.0,14.0/16.0);
          uv2= glm::vec2(3.0/16.0,14.0/16.0);
          uv3= glm::vec2(3.0/16.0,15.0/16.0);
          uv4= glm::vec2(2.0/16.0,15.0/16.0);
      }else if(t == GOLD){
          uv1= glm::vec2(0.0/16.0,13.0/16.0);
          uv2= glm::vec2(1.0/16.0,13.0/16.0);
          uv3= glm::vec2(1.0/16.0,14.0/16.0);
          uv4= glm::vec2(0.0/16.0,14.0/16.0);
      }
      else{
          uv1= glm::vec2(2.0/16.0,15.0/16.0);
          uv2= glm::vec2(3.0/16.0,15.0/16.0);
          uv3= glm::vec2(3.0/16.0,16.0/16.0);
          uv4= glm::vec2(2.0/16.0,16.0/16.0);
      }

      uv1 = uv1 + glm::vec2(EPSILON,EPSILON);
      uv2 = uv2 + glm::vec2(-EPSILON,EPSILON);
      uv3 = uv3 + glm::vec2(-EPSILON,-EPSILON);
      uv4 = uv4 + glm::vec2(EPSILON,-EPSILON);



      uvs.push_back(uv1);
      uvs.push_back(uv2);
      uvs.push_back(uv3);
      uvs.push_back(uv4);



}

void Chunk::push_power(std::vector<float> &powers, BlockType t){
    if(t == GRASS){
        powers.push_back(4.0);
        powers.push_back(4.0);
        powers.push_back(4.0);
        powers.push_back(4.0);

    }else if(t == DIRT){
        powers.push_back(2.0);
        powers.push_back(2.0);
        powers.push_back(2.0);
        powers.push_back(2.0);
    }else if(t == STONE){
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
    }else if(t == WATER){
        powers.push_back(7.0);
        powers.push_back(7.0);
        powers.push_back(7.0);
        powers.push_back(7.0);
    }else if(t == LAVA){
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
    }else if(t == WOOD){
        powers.push_back(2.0);
        powers.push_back(2.0);
        powers.push_back(2.0);
        powers.push_back(2.0);
    }else if(t == LEAF){
        powers.push_back(4.0);
        powers.push_back(4.0);
        powers.push_back(4.0);
        powers.push_back(4.0);
    }else if(t == BEDROCK){
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
        powers.push_back(3.0);
    }else if(t == ICE){
        powers.push_back(9.0);
        powers.push_back(9.0);
        powers.push_back(9.0);
        powers.push_back(9.0);
    }else{
            powers.push_back(3.0);
            powers.push_back(3.0);
            powers.push_back(3.0);
            powers.push_back(3.0);
    }
}


