#pragma once

#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

/**
 * Defines a subregion within a texture that the texture operation
 * will act on.
 */ 
class ActiveArea {
public:
    int activeCenterX;
    int activeCenterY;
    int activeWidth;
    int activeHeight;

    ActiveArea() {

    }

    ActiveArea(int activeCenterX, int activeCenterY, int activeWidth, int activeHeight) {
        this->activeCenterX = activeCenterX;
        this->activeCenterY = activeCenterY;
        this->activeWidth = activeWidth;
        this->activeHeight = activeHeight;
    }
};

class TextureOperation {
public:
    int textureWidth;
    int textureHeight;
    GLuint shaderProgram;
    Quad quadVAO;
    FBOWrapper fbo;

    bool doClear;

    // Offset for only uppdating part of texture
    ActiveArea activeArea;

    TextureOperation() {

    }
    TextureOperation(int textureWidth, int textureHeight, GLuint shaderProgram) {
        this->textureWidth = textureWidth;
        this->textureHeight = textureHeight;
        this->shaderProgram = shaderProgram;
    
        activeArea = ActiveArea(0, 0, textureWidth, textureHeight);

        fbo = createFrameBufferSingleTexture(textureWidth, textureHeight);

        doClear = true;
    }

    virtual void bindUniforms() {
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(textureWidth, textureHeight, 1));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-activeArea.activeCenterX, -activeArea.activeCenterY, 0));
        glm::mat4 scale2 = glm::scale(glm::mat4(1.0f), glm::vec3(1/(activeArea.activeWidth/2.0f), 1/(activeArea.activeHeight/2.0f), 1));
        glm::mat4 modelToWorld = scale2 * translate * scale;
        glm::mat4 identity = glm::mat4(1.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(identity));

        glUniform1i(glGetUniformLocation(shaderProgram, "textureWidth"), textureWidth);
        glUniform1i(glGetUniformLocation(shaderProgram, "textureHeight"), textureHeight);

        glUniform1i(glGetUniformLocation(shaderProgram, "activeHeight"), activeArea.activeHeight);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeWidth"), activeArea.activeWidth);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeCenterX"), activeArea.activeCenterX);
        glUniform1i(glGetUniformLocation(shaderProgram, "activeCenterY"), activeArea.activeCenterY);
    }

    virtual void execute(GLuint textureInput1, GLuint textureInput2) {
        glViewport(activeArea.activeCenterX - activeArea.activeWidth/2.0f + textureWidth/2.0f, activeArea.activeCenterY - activeArea.activeHeight/2.0f + textureHeight/2.0f, activeArea.activeWidth, activeArea.activeHeight);
        glUseProgram(shaderProgram);
        
        this->bindUniforms();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureInput1);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureInput2);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo.fboId);
        glClearColor(0, 0.1, 0.5, 1);
        glDisable(GL_DEPTH_TEST);

        // glClear(GL_DEPTH_BUFFER_BIT);
        
        if (doClear) {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glBindVertexArray(quadVAO.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVAO.indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);   
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glEnable(GL_DEPTH_TEST);
    }

    virtual GLuint getTextureResult() {
        return fbo.textureId;
    }
};

class PingPongTextureOperation : public TextureOperation {
public:
    int timesRepeat;
    FBOWrapper extraFBO;

    PingPongTextureOperation() {

    }
    
    PingPongTextureOperation(int textureWidth, int textureHeight, GLuint shaderProgram, int timesRepeat) :  TextureOperation(textureWidth, textureHeight, shaderProgram) {
        this->timesRepeat = timesRepeat;
        extraFBO = createFrameBufferSingleTexture(textureWidth, textureHeight);
    }

    // The first texture is pong ponged
    void execute(GLuint textureInput1, GLuint textureInput2) override {
        FBOWrapper fboOrig = fbo;
        GLuint nextInputTexture = textureInput1; 
        for (int i = 0; i < timesRepeat; i++) {
            TextureOperation::execute(nextInputTexture, textureInput2);
            if (fbo.fboId == fboOrig.fboId) {
                fbo = extraFBO;
                nextInputTexture = fboOrig.textureId;
            } else {
                fbo = fboOrig;
                nextInputTexture = extraFBO.textureId;
            }
        }
        fbo = fboOrig;
    }

    GLuint getTextureResult() override {
        if (timesRepeat % 2 == 0) {
            return extraFBO.textureId;
        } else {
            return fbo.textureId;
        }
    }
};

class CalculateNormalsOperation : public TextureOperation {
public:
    float heightScale;
        
    CalculateNormalsOperation(int textureWidth, int textureHeight, GLuint shaderProgram, float heightScale) :  TextureOperation(textureWidth, textureHeight, shaderProgram) {
        this->heightScale = heightScale;
    }

    void bindUniforms() override {
        glUniform1f(glGetUniformLocation(shaderProgram, "heightScale"), heightScale);
        TextureOperation::bindUniforms();
    }
};


class MultiplyOperation : public TextureOperation {
public:
    float factor;
        
    MultiplyOperation(int textureWidth, int textureHeight, GLuint shaderProgram, float factor) :  TextureOperation(textureWidth, textureHeight, shaderProgram) {
        this->factor = factor;
    }

    void bindUniforms() override {
        glUniform1f(glGetUniformLocation(shaderProgram, "factor"), factor);
        TextureOperation::bindUniforms();
    }
};