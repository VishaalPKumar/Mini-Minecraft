#include "pointer.h"


static const float SIDE_LENGTH = 0.025f;

Pointer::Pointer(OpenGLContext* context)
    : Drawable(context)
{}

Pointer::~Pointer()
{}



void Pointer::createVBOdata()
{
    GLuint idx[4] = {0, 1, 2, 3};

    glm::vec4 pos[4] = {
            glm::vec4(-SIDE_LENGTH, 0, 0, 1),
            glm::vec4(SIDE_LENGTH, 0, 0, 1),
            glm::vec4(0, -SIDE_LENGTH, 0, 1),
            glm::vec4(0, SIDE_LENGTH, 0, 1)
    };

    glm::vec4 col[4] = {
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 1, 1, 1),
        glm::vec4(1, 1, 1, 1),
    };

    indexCounts[INDEX] = 4;

    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generateBuffer(POSITION);
    bindBuffer(POSITION);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateBuffer(COLOR);
    bindBuffer(COLOR);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

GLenum Pointer::drawMode()
{
    return GL_LINES;
}
