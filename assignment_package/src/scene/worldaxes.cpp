#include "worldaxes.h"

void WorldAxes::create()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    glm::vec4 pos[6] = {glm::vec4(32,129,32,1), glm::vec4(40,129,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,137,32,1),
                        glm::vec4(32,129,32,1), glm::vec4(32,129,40,1)};
//    glm::vec4 pos[6] = {glm::vec4(0,0,0,1), glm::vec4(8,0,0,1),
//                        glm::vec4(0,0,0,1), glm::vec4(0,8,0,1),
//                        glm::vec4(0,0,0,1), glm::vec4(0,0,8,1)};
    glm::vec4 col[6] = {glm::vec4(1,0,0,1), glm::vec4(1,0,0,1),
                        glm::vec4(0,1,0,1), glm::vec4(0,1,0,1),
                        glm::vec4(0,0,1,1), glm::vec4(0,0,1,1)};

    count = 6;

    glm::vec4 posNorCol[24];
    for(int i = 0; i < 6; ++i)
    {
        posNorCol[i * 4] = pos[i];
        posNorCol[i * 4 + 1] = glm::vec4{0.f, 0.f, 1.f, 0.f};
        posNorCol[i * 4 + 2] = col[i];
        posNorCol[i * 4 + 3] = glm::vec4{0.f, 0.f, 0.f, 0.f};
    }

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(glm::vec4), posNorCol, GL_STATIC_DRAW);
}

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
