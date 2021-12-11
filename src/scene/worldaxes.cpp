#include "worldaxes.h"

void WorldAxes::create()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    std::vector<glm::vec4> vert;
    glm::vec4 pos[6] = {glm::vec4(32,129,32,1), glm::vec4(40,129,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,137,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,129,40,1)};
    glm::vec4 col[6] = {glm::vec4(1,0,0,1), glm::vec4(1,0,0,1),
                        glm::vec4(0,1,0,1), glm::vec4(0,1,0,1),
                        glm::vec4(0,0,1,1), glm::vec4(0,0,1,1)};

    count = 6;
    for(int i = 0; i < 6; i++)
    {
        vert.push_back(pos[i]);
        vert.push_back(glm::vec4(0, 0, 0, 1));
        vert.push_back(col[i]);
    }
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generateVert();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufVert);
    context->glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(glm::vec4), vert.data(), GL_STATIC_DRAW);
 }

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
