#pragma once
#include <QOpenGLFunctions_3_3_Core>
#include "openglcontext.h"

class Texture
{
private:
    // QOpenGLFunctions_3_3_Core* glContext;
    OpenGLContext* glContext;
    GLuint m_textureHandle;
    GLuint m_TextureSlot;

public:
    Texture(OpenGLContext* context, GLuint textureSlot);
    ~Texture();

    void create(const char *texturePath,
                GLenum internalFormat = GL_RGBA,
                GLenum format = GL_BGRA);
    void bufferPixelData(unsigned int width, unsigned int height,
                         GLenum internalFormat, GLenum format, GLvoid *pixels);
    void bind(GLuint texSlot);
    void destroy();

    GLuint getHandle() const;
    GLuint getTextureSlot() const;


};
