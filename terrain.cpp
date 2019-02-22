#include <algorithm>

#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "texture.cpp"

class TerrainData {
public:
    int numIndices;
    GLuint indexBuffer;
    GLuint vao;
};

namespace TerrainUtils {
    
    // const float quadSize = 0.5f;

    // const float vertexCoordinates[] = {
    //     -quadSize,  0,  quadSize,
    //     quadSize, 0,  quadSize,
    //     quadSize, 0,  -quadSize,
    //     -quadSize, 0,  -quadSize,
    // };

    // const float textureCoordinates[] = {
    //     0.0f, 0.0f,
    //     1.0f, 0.0f,
    //     1.0f, 1.0f,
    //     0.0f, 1.0f,
    // };

    // const float normals[] = {
    //     0.0f, 1.0f, 0.0f,
    //     0.0f, 1.0f, 0.0f,
    //     0.0f, 1.0f, 0.0f,
    //     0.0f, 1.0f, 0.0f,
    // };

    // unsigned int indices[] {
    //     0, 1, 3,
    //     1, 2, 3,
    // };

    float* getFlatHeightmap(int size) {
        float* heightmap = new float[size * size];
        std::fill_n(heightmap, size * size, 0);
        for (int i = 0; i < size * size; i++) {
            heightmap[i] = 0;
        }
        return heightmap;
    }

    // Assumes square
    TerrainData meshFromHeightpoints(float* heightmap, int size) {
        int width = size;
        int height = size;
        float scaleX = 1;
        float scaleY = 1;
        float scaleZ = 1;
        float textureScale = 1;
        
        int numVertices = width * height;
        float *positions = new float[numVertices * 3];
        float *normals = new float[numVertices * 3];
        float *textureCoordinates = new float[numVertices * 2];
        int indicesArraySize = (width - 1) * (height - 1) * 6;
        int *indices = new int[indicesArraySize];

        for (int z = 0; z < height; z++) {
            for (int x = 0; x < width; x++) {
                positions[(z * width + x) * 3] = x * scaleX;
                positions[(z * width + x) * 3 + 1] = heightmap[width * z + x] * scaleY;
                positions[(z * width + x) * 3 + 2] = z * scaleZ;
                textureCoordinates[(z * width + x) * 2] = ((float)x / (float)(width - 1)) * textureScale;
                textureCoordinates[(z * width + x) * 2 + 1] = (1 - (float)z / (float)(height - 1)) * textureScale;

                // Make all normals point up
                normals[(z * width + x) * 3] = 0;
                normals[(z * width + x) * 3 + 1] = 1;
                normals[(z * width + x) * 3 + 2] = 0;

                // Indices
                if (x != width - 1 && z != height - 1) {
                    int index = z * width + x;
                    int arrayIndex = z * (width - 1) + x;
                    indices[arrayIndex * 6] = index;
                    indices[arrayIndex * 6 + 1] = index + 1;
                    indices[arrayIndex * 6 + 2] = index + width;
                    indices[arrayIndex * 6 + 3] = index + 1;
                    indices[arrayIndex * 6 + 4] = index + width + 1;
                    indices[arrayIndex * 6 + 5] = index + width;
                }
            }
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), positions, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboNormals = 0;
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(float), normals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboTex = 0;
        glGenBuffers(1, &vboTex);
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 2 * sizeof(float), textureCoordinates, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboIndices;
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize * sizeof(unsigned int), indices, GL_STATIC_DRAW);

        TerrainData tr;
        tr.indexBuffer = vboIndices;
        tr.numIndices = indicesArraySize;
        tr.vao = vao;
        return tr;
    }

}

class Terrain {
public:
    glm::vec3 position;
    glm::vec3 scale;
    GLuint vao;
    GLuint indexBuffer;
    GLuint textureId;
    GLuint heightmap;
    GLuint normalmap;
    int numIndices;

    Terrain() {
        int numGridsX = 31;
        TerrainData tr =  TerrainUtils::meshFromHeightpoints(TerrainUtils::getFlatHeightmap(numGridsX), numGridsX);
        vao = tr.vao;
        indexBuffer = tr.indexBuffer;
        numIndices = tr.numIndices;
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
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        glActiveTexture(GL_TEXTURE1);
	    glBindTexture(GL_TEXTURE_2D, heightmap);
        glUniform1i(glGetUniformLocation(shaderProgram, "heightmap"), 1);

        glActiveTexture(GL_TEXTURE2);
	    glBindTexture(GL_TEXTURE_2D, normalmap);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalmap"), 2);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, 0);
    }
};