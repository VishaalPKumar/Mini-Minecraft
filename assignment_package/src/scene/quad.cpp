#include "quad.h"

static const int QUAD_IDX_COUNT = 6;
static const int QUAD_VERT_COUNT = 4;

Quad::Quad(OpenGLContext* context)
    : Drawable(context)
{}

Quad::~Quad()
{}



void Quad::createVBOdata()
{
    std::vector<glm::vec4> glPos { glm::vec4(-1,-1, 1, 1),
                                 glm::vec4( 1,-1, 1, 1),
                                 glm::vec4( 1, 1, 1, 1),
                                 glm::vec4(-1, 1, 1, 1) };

    std::vector<glm::vec2> glUV { glm::vec2(0,0),
                                glm::vec2(1,0),
                                glm::vec2(1,1),
                                glm::vec2(0,1) };

    std::vector<GLuint> glIndex {0,1,2,0,2,3};

    indexCounts[INDEX] = QUAD_IDX_COUNT;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, QUAD_IDX_COUNT * sizeof(GLuint), glIndex.data(), GL_STATIC_DRAW);

    generateBuffer(POSITION);
    bindBuffer(POSITION);
    mp_context->glBufferData(GL_ARRAY_BUFFER, QUAD_VERT_COUNT * sizeof(glm::vec4), glPos.data(), GL_STATIC_DRAW);

    generateBuffer(UV);
    bindBuffer(UV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, QUAD_VERT_COUNT * sizeof(glm::vec2), glUV.data(), GL_STATIC_DRAW);
}
