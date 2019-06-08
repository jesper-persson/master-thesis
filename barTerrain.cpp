#include <algorithm>

#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "texture.cpp"

class BarTerrainData {
public:
    int numIndices;
    GLuint indexBuffer;
    GLuint vao;
};

namespace BarTerrainUtils {
    float* getFlatHeightmap(int size) {
        float* heightmap = new float[size * size];
        std::fill_n(heightmap, size * size, 0);
        for (int i = 0; i < size * size; i++) {
            heightmap[i] = 0;
        }
        return heightmap;
    }

    // Assumes square
    BarTerrainData meshFromHeightpoints(int heightmapSize) {
        int size = heightmapSize;
        float scale = 1/(float)(size);
        
        int numVertices = size * size * 8;
        float *positions = new float[numVertices * 3];
        float *normals = new float[numVertices * 3];
        float *textureCoordinates = new float[numVertices * 2];
        int indicesArraySize = size * size * 18; // was 6
        int *indices = new int[indicesArraySize];

        for (int z = 0; z < size; z++) {
            for (int x = 0; x < size; x++) {
                // For each bar

                int barIndex = z * size + x;
                int elementsPerVertex = 3;
                int verticesPerBar = 8;
                int vertexStartIndex = barIndex * verticesPerBar * elementsPerVertex;

                int elementsPerTexCoord = 2;
                int texCoordStartIndex = barIndex * verticesPerBar * elementsPerTexCoord;
                
                float xCoordPrev = (x-1) / float(size - 1);
                float yCoordPrev = (z-1) / float(size - 1);
                float xCoordNext = (x+1) / float(size - 1);
                float yCoordNext = (z+1) / float(size - 1);
                float xCoord = x / float(size - 1);
                float yCoord = z / float(size - 1);

                // Vertex 1
                positions[vertexStartIndex + 0] = x * scale - 0.5f;
                positions[vertexStartIndex + 1] = 0;
                positions[vertexStartIndex + 2] = z * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 0] = xCoord;
                textureCoordinates[texCoordStartIndex + 1] = yCoord;

                // Vertex 2
                positions[vertexStartIndex + 3] = (x + 1) * scale - 0.5f;
                positions[vertexStartIndex + 4] = 0;
                positions[vertexStartIndex + 5] = z * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 2] = xCoord;
                textureCoordinates[texCoordStartIndex + 3] = yCoord;

                // Vertex 3
                positions[vertexStartIndex + 6] = (x + 1) * scale - 0.5f;
                positions[vertexStartIndex + 7] = 0;
                positions[vertexStartIndex + 8] = (z + 1) * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 4] = xCoord;
                textureCoordinates[texCoordStartIndex + 5] = yCoord;

                // Vertex 4
                positions[vertexStartIndex + 9] = x * scale - 0.5f;
                positions[vertexStartIndex + 10] = 0;
                positions[vertexStartIndex + 11] = (z + 1) * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 6] = xCoord;
                textureCoordinates[texCoordStartIndex + 7] = yCoord;

                // Vertex 5 - right edge
                positions[vertexStartIndex + 12] = (x + 1) * scale - 0.5f;
                positions[vertexStartIndex + 13] = 0;
                positions[vertexStartIndex + 14] = z * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 8] = xCoordNext;
                textureCoordinates[texCoordStartIndex + 9] = yCoord;
                
                // Vertex 6
                positions[vertexStartIndex + 15] = (x + 1) * scale - 0.5f;
                positions[vertexStartIndex + 16] = 0;
                positions[vertexStartIndex + 17] = (z + 1) * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 10] = xCoordNext;
                textureCoordinates[texCoordStartIndex + 11] = yCoord;
                
                // Vertex 7 - down edge
                positions[vertexStartIndex + 18] = (x + 1) * scale - 0.5f;
                positions[vertexStartIndex + 19] = 0;
                positions[vertexStartIndex + 20] = (z + 1) * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 12] = xCoord;
                textureCoordinates[texCoordStartIndex + 13] = yCoordNext;

                // Vertex 8
                positions[vertexStartIndex + 21] = x * scale - 0.5f;
                positions[vertexStartIndex + 22] = 0;
                positions[vertexStartIndex + 23] = (z + 1) * scale - 0.5f;
                textureCoordinates[texCoordStartIndex + 14] = xCoord;
                textureCoordinates[texCoordStartIndex + 15] = yCoordNext;

                int barStartVertexIndex = barIndex * 8;
                int barStartVertexIndexNext = barIndex * 8 + 8;
                int barStartVertexIndexNextRow = (barIndex + size) * 8;

                // Indices
                int startIndex = barIndex * 18;
                indices[startIndex + 0] = barStartVertexIndex + 2;
                indices[startIndex + 1] = barStartVertexIndex + 1;
                indices[startIndex + 2] = barStartVertexIndex; 
                indices[startIndex + 3] = barStartVertexIndex + 2;
                indices[startIndex + 4] = barStartVertexIndex;
                indices[startIndex + 5] = barStartVertexIndex + 3;

                indices[startIndex + 6] = barStartVertexIndex + 5;
                indices[startIndex + 7] = barStartVertexIndex + 4;
                indices[startIndex + 8] = barStartVertexIndex + 1;
                indices[startIndex + 9] = barStartVertexIndex + 5;
                indices[startIndex + 10] = barStartVertexIndex + 1;
                indices[startIndex + 11] = barStartVertexIndex + 2;

                indices[startIndex + 12] = barStartVertexIndex + 6;
                indices[startIndex + 13] = barStartVertexIndex + 2;
                indices[startIndex + 14] = barStartVertexIndex + 3;
                indices[startIndex + 15] = barStartVertexIndex + 7;
                indices[startIndex + 16] = barStartVertexIndex + 6;
                indices[startIndex + 17] = barStartVertexIndex + 3;
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

        BarTerrainData tr;
        tr.indexBuffer = vboIndices;
        tr.numIndices = indicesArraySize;
        tr.vao = vao;
        return tr;
    }
}

class BarTerrain {
public:
    glm::vec3 position;
    glm::vec3 scale;
    GLuint vao;
    GLuint indexBuffer;
    GLuint textureId;
    GLuint heightmap;
    GLuint normalmapMacro;
    GLuint normalmap;
    int numIndices;

    int heightColumnScale;

    BarTerrain(int heightmapSize) {
        BarTerrainData tr =  BarTerrainUtils::meshFromHeightpoints(heightmapSize);
        vao = tr.vao;
        indexBuffer = tr.indexBuffer;
        numIndices = tr.numIndices;
        textureId = loadPNGTexture("resources/sample.png");
        position = glm::vec3(0, 0, 0);
        scale = glm::vec3(1, 1, 1);
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection, bool useFloatTexture, GLuint floatHeightmap) {
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
        if (useFloatTexture) {
	        glBindTexture(GL_TEXTURE_2D, floatHeightmap);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
	        glBindTexture(GL_TEXTURE_2D, heightmap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glUniform1i(glGetUniformLocation(shaderProgram, "heightmap"), 1);

        glActiveTexture(GL_TEXTURE2);
	    glBindTexture(GL_TEXTURE_2D, normalmapMacro);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalMapMacro"), 2);

        glActiveTexture(GL_TEXTURE3);
	    glBindTexture(GL_TEXTURE_2D, normalmap);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 3);

        glUniform1i(glGetUniformLocation(shaderProgram, "normalMapRepeat"), 8);
        
        glUniform1i(glGetUniformLocation(shaderProgram, "heightColumnScale"), heightColumnScale);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, 0);
    }
};