#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

/**
 * The viewport needs to be the same size as the textures, since we want the fragment shader to be called
 * for each pixel.
 */
void calculateTextureDiff(GLuint shaderProgram, Quad quad, FBOWrapper fbo, GLuint textureId1, GLuint textureId2) {
    glUseProgram(shaderProgram);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2, 2, 1));
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 modelToWorld = translate * scale;

    glm::mat4 identity = glm::mat4(1.0f);

    // glBindBuffer(GL_ARRAY_BUFFER, quad.vao);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(identity));
    
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