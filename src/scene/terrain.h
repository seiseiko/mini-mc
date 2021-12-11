#pragma once
#include <QList>
#include <la.h>
#include <drawable.h>
#include <smartpointerhelp.h>
// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
#define WIDTH 16
#define HEIGHT 256
#define DEPTH 16
#define TERRAIN_SIZE 64
#define CHUNK_SIZE 64

enum BlockType : unsigned char
{

    EMPTY, GRASS, DIRT, STONE, WATER,LAVA,WOOD,ICE,LEAF,BEDROCK,SAND, SNOW,GOLD

};

enum BlockSide : unsigned char{
    FRONT,BACK,UP,DOWN,LEFT,RIGHT
};

class Chunk;
class Terrain
{
public:
    Terrain();
    BlockType m_blocks[64][256][64];                    // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.
    std::vector<uPtr<Chunk>> chunk_list;
    void update_adjacent();
    std::map<long long int, Chunk*> mapping;
    void create_update();
    OpenGLContext* t_context;
    int minX;
    int maxX;
    int minZ;
    int maxZ;
    BlockType e = EMPTY;

    void CreateTestScene();

    glm::ivec3 dimensions;
    BlockType& getBlockAt2(int x, int y, int z);
    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t,
                    OpenGLContext* context); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
};


// Chunk Class

class Chunk: public Drawable
{
public:
    Chunk(OpenGLContext* context);
    ~Chunk();

    BlockType type[WIDTH * DEPTH * HEIGHT];  // 1D array of BlockType,  16 x 256 x 16
    BlockType get_type(int x, int y, int z) const;
    BlockType& get_type2(int x, int y, int z);
    glm::vec4 color_list[5];
    Chunk* front;
    Chunk* back;
    Chunk* left;
    Chunk* right; // adjacent chunks


    glm::vec3 pos; // minimum corner's world position

    void create();
    void destroy(); // Frees the VBOs of the Drawable.
    void push_face_data(glm::vec4 pos1, glm::vec4 pos2,
                        glm::vec4 pos3, glm::vec4 pos4,
                        std::vector<glm::vec4>& vert,std::vector<glm::vec2>& uvs,std::vector<GLuint>& idx,
                        glm::vec4 col,BlockType t,BlockSide side);
    void push_power(std::vector<float>& powers,BlockType t);
    void create_update();


};
