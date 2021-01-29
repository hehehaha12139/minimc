#include "overlayquad.h"

const static glm::vec4 BLUE(0.f, 0.f, 1.f, 1.f);
const static glm::vec4 RED(1.f, 0.f, 0.f, 1.f);

OverlayQuad::OverlayQuad(OpenGLContext* context, char t)
    : Drawable(context), type(t)
{}

// Creates VBO data to make a visual representation of the currently selected Vertex
void OverlayQuad::create()
{
    GLuint idx[6] = {0, 1, 2, 0, 2, 3};
    glm::vec4 pos[4] = {glm::vec4(-1.f, -1.f, 1.f, 1.f),
                        glm::vec4(1.f, -1.f, 1.f, 1.f),
                        glm::vec4(1.f, 1.f, 1.f, 1.f),
                        glm::vec4(-1.f, 1.f, 1.f, 1.f)};

    glm::vec4 col = type == 'b' ? BLUE : RED;
    glm::vec4 posNorCol[16];
    for(int i = 0; i < 4; ++i)
    {
        posNorCol[i * 4] = pos[i];
        posNorCol[i * 4 + 1] = glm::vec4{0.f, 0.f, 1.f, 0.f};
        posNorCol[i * 4 + 2] = col;
        posNorCol[i * 4 + 3] = glm::vec4{0.f, 0.f, 0.f, 0.f};
    }

    count = 6;
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(glm::vec4), posNorCol, GL_STATIC_DRAW);
}
