#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

/**
 * The viewport needs to be the same size as the textures, since we want the fragment shader to be called
 * for each pixel.
 */
void textureOperation(GLuint shaderProgram, Quad quad, FBOWrapper fbo, GLuint textureId1, GLuint textureId2, int textureSize) {
    glUseProgram(shaderProgram);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2, 2, 1));
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 modelToWorld = translate * scale;

    glm::mat4 identity = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(identity));

    glUniform1i(glGetUniformLocation(shaderProgram, "textureSize"), textureSize);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId1);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureId2);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fboId);
    glClearColor(0.5, 0.1, 0.5, 1);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(quad.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);   
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// times should be multiple if 2, so that the final result is in fbo2
// only works for single texture operations for now
void textureOperationRepeat(int times, GLuint shaderProgram, Quad quad, FBOWrapper fbo1, FBOWrapper fbo2, GLuint textureId1, GLuint textureId2, int textureSize) {
    FBOWrapper nextOutputFBO = fbo1;
    GLuint nextInputTexture = textureId1; 
    for (int i = 0; i < times; i++) {
        textureOperation(shaderProgram, quad, nextOutputFBO, nextInputTexture, 0, textureSize);
        if (nextOutputFBO.fboId == fbo1.fboId) {
            nextOutputFBO = fbo2;
            nextInputTexture = fbo1.textureId;
        } else {
            nextOutputFBO = fbo1;
            nextInputTexture = fbo2.textureId;
        }
    }
}