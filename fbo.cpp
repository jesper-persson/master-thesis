#pragma once

#include <iostream>

#include "glew.h"

using namespace std;

enum class TextureFormat {
    RGBA16F, R32UI, R32I, RGBA32UI, RGBA32I, RG32UI
};

class FBOWrapper
{
public:
    GLuint fboId;
    GLuint textureId;

    void setOutputTexture(GLuint textureId) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureId, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        this->textureId = textureId;
    }
};

FBOWrapper createFBOForDepthTexture(int textureWidth, int textureHeight)
{
    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureWidth, textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
    glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Could not create frame buffer" << std::endl;
    }

    FBOWrapper fbo;
    fbo.fboId = fboId;
    fbo.textureId = depthTexture;
    return fbo;
}

// textureInterpolation: 0=nearest, 1=linear
FBOWrapper createFrameBufferSingleTexture(int textureWidth, int textureHeight, TextureFormat textureFormat)
{
    GLuint fboId = 0;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    GLuint renderedTexture;
    {
        glGenTextures(1, &renderedTexture);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);

        const int w = textureWidth;
        const int h = textureHeight;

        if (textureFormat == TextureFormat::RGBA16F) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, 0);
        } else if (textureFormat == TextureFormat::R32UI) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, w, h, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
        } else if (textureFormat == TextureFormat::R32I) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, w, h, 0, GL_RED_INTEGER, GL_INT, 0);
        } else if (textureFormat == TextureFormat::RGBA32UI) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, w, h, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 0);
        } else if (textureFormat == TextureFormat::RG32UI) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32UI, w, h, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, 0);
        } else if (textureFormat == TextureFormat::RGBA32I) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, w, h, 0, GL_RGBA_INTEGER, GL_INT, 0);
        } else {
            cerr << "Unsupported format" << endl;
        }

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    }

    // Depth
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Only if multible draw buffers?
    // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Could not create frame buffer" << std::endl;
    }

    FBOWrapper fbo{};
    fbo.fboId = fboId;
    fbo.textureId = renderedTexture;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}