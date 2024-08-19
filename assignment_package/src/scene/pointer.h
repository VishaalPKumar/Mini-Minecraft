#pragma once
#include "drawable.h"

class Pointer : public Drawable {
public:
    Pointer(OpenGLContext* context);
    ~Pointer();

    void createVBOdata() override;
    GLenum drawMode() override;
};
