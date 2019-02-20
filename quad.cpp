#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "texture.cpp"

namespace QuadUtils {
    const float quadSize = 0.5f;

    const float vertexCoordinates[] = {
        -quadSize,  quadSize,  0,
        quadSize,  quadSize,  0,
        quadSize, -quadSize,  0,
        -quadSize, -quadSize,  0
    };

    const float textureCoordinates[] = {
        0.0f, 1.0f,  0.0f,
        1.0f, 1.0f,  0.0f,
        1.0f, 0.0f,  0.0f,
        0.0f, 0.0f,  0.0f,
    };

    const float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    };

    unsigned int indices[] {
        0, 1, 3,
        1, 2, 3,
    };

    GLuint createVAO() {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), vertexCoordinates, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboNormals = 0;
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), normals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboTex = 0;
        glGenBuffers(1, &vboTex);
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), textureCoordinates, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        return vao;
    }

    GLuint createIndexBuffer() {
        GLuint vboIndices;
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        return vboIndices;
    }

}

class Quad {
public:
    glm::vec3 position;
    glm::vec3 scale;
    GLuint vao;
    GLuint indexBuffer;
    GLuint textureId;
    Quad() {
        vao = QuadUtils::createVAO();
        indexBuffer = QuadUtils::createIndexBuffer();
        textureId = loadPNGTexture("images/sample.png");
        position = glm::vec3(0, 0, 0);
        scale = glm::vec3(1, 1, 1);
    }
    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        glUseProgram(shaderProgram);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 modelToWorld = translate * scale;

        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(worldToCamera));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), GL_TEXTURE0);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};