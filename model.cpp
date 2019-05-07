#include "glew.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#include "texture.cpp"

void calculateTangentsAndBiTangents(int numVertices, const float *vertexCoords, const float *textureCoords, float *tangents, float *bitangents) {
	// Each iteration processes one triangle
	for (int i = 0; i < numVertices * 3; i += 9) {
		glm::vec3 vertexCoordA = glm::vec3(vertexCoords[i], vertexCoords[i + 1], vertexCoords[i + 2]);
		glm::vec3 vertexCoordB = glm::vec3(vertexCoords[i + 3], vertexCoords[i + 4], vertexCoords[i + 5]);
		glm::vec3 vertexCoordC = glm::vec3(vertexCoords[i + 6], vertexCoords[i + 7], vertexCoords[i + 8]);

		glm::vec3 vertexDiff1 = glm::normalize(vertexCoordB - vertexCoordA);
		glm::vec3 vertexDiff2 = glm::normalize(vertexCoordC - vertexCoordA);

		glm::vec2 texCoord1 = glm::vec2(textureCoords[i / 3 * 2], textureCoords[i / 3 * 2 + 1]);
		glm::vec2 texCoord2 = glm::vec2(textureCoords[i / 3 * 2 + 2], textureCoords[i / 3 * 2 + 3]);
		glm::vec2 texCoord3 = glm::vec2(textureCoords[i / 3 * 2 + 4], textureCoords[i / 3 * 2 + 5]);

		glm::vec2 texureDiff1 = glm::normalize(texCoord2 - texCoord1);
		glm::vec2 textureDiff2 = glm::normalize(texCoord3 - texCoord1);

		float r = 1.0f / (texureDiff1.x * textureDiff2.y - texureDiff1.y * textureDiff2.x);
		glm::vec3 tangent = glm::normalize((vertexDiff1 * textureDiff2.y - vertexDiff2 * texureDiff1.y) * r);
		glm::vec3 bitangent = glm::normalize((vertexDiff2 * texureDiff1.x - vertexDiff1 * textureDiff2.x) * r);

		tangents[i] = tangent.x;
		tangents[i + 1] = tangent.y;
		tangents[i + 2] = tangent.z;
		tangents[i + 3] = tangent.x;
		tangents[i + 4] = tangent.y;
		tangents[i + 5] = tangent.z;
		tangents[i + 6] = tangent.x;
		tangents[i + 7] = tangent.y;
		tangents[i + 8] = tangent.z;
		bitangents[i] = bitangent.x;
		bitangents[i + 1] = bitangent.y;
		bitangents[i + 2] = bitangent.z;
		bitangents[i + 3] = bitangent.x;
		bitangents[i + 4] = bitangent.y;
		bitangents[i + 5] = bitangent.z;
		bitangents[i + 6] = bitangent.x;
		bitangents[i + 7] = bitangent.y;
		bitangents[i + 8] = bitangent.z;
	}
}

namespace ModelUtils {
    GLuint createVAO(float* vertexCoordinates, int vertexCoordinatesSize,
                     float* normals, int normalsSize,
                     float* textureCoordinates, int textureCoordinatesSize) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexCoordinatesSize * sizeof(float), vertexCoordinates, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboNormals = 0;
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, normalsSize * sizeof(float), normals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboTex = 0;
        glGenBuffers(1, &vboTex);
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glBufferData(GL_ARRAY_BUFFER, textureCoordinatesSize * sizeof(float), textureCoordinates, GL_STATIC_DRAW); // 3 should be 2?
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL); // 3 should be 2?

        float* tangents = new float[vertexCoordinatesSize];
        float* bitangents = new float[vertexCoordinatesSize];
        calculateTangentsAndBiTangents(vertexCoordinatesSize / 3, vertexCoordinates, textureCoordinates, tangents, bitangents);

        GLuint vboTangents = 0;
        glGenBuffers(1, &vboTangents);
        glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
        glBufferData(GL_ARRAY_BUFFER, vertexCoordinatesSize * sizeof(float), tangents, GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vboBitangents = 0;
        glGenBuffers(1, &vboBitangents);
        glBindBuffer(GL_ARRAY_BUFFER, vboBitangents);
        glBufferData(GL_ARRAY_BUFFER, vertexCoordinatesSize * sizeof(float), bitangents, GL_STATIC_DRAW);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);

        return vao;
    }

    GLuint createIndexBuffer(int* indices, int indicesSize) {
        GLuint vboIndices;
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        return vboIndices;
    }
}

class Model {
public:
    glm::vec3 position;
    glm::vec3 scale;
    glm::mat4 rotation;
    glm::vec3 forward;
    glm::vec3 up;

    GLuint vao;
    GLuint indexBuffer;
    GLuint textureId;
    int numIndices;
    int startIndex;

    GLuint normalMap;
    bool useNormalMapping;

    Model() {
        position = glm::vec3(0, 0, 0);
        scale = glm::vec3(1, 1, 1);
        rotation = glm::mat4(1.0f);
        useNormalMapping = false;
        forward = glm::vec3(0, 0, 1);
        up = glm::vec3(0, 1, 0);
        startIndex = 0;
    }

    void render(GLuint shaderProgram, glm::mat4 worldToCamera, glm::mat4 projection) {
        glUseProgram(shaderProgram);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);

        // glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotation = glm::lookAt(position, position + forward * 1.0f, up);
        rotation = glm::inverse(rotation);
        glm::mat4 modelToWorld = rotation * scale;

        glBindBuffer(GL_ARRAY_BUFFER, vao);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelToWorld"), 1, GL_FALSE, glm::value_ptr(modelToWorld));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldToCamera"), 1, GL_FALSE, glm::value_ptr(worldToCamera));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        if (useNormalMapping) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalMap);
            glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);
            glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMapping"), 1);
            glUniform1i(glGetUniformLocation(shaderProgram, "normalMapRepeat"), 1);
        } else {
            glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMapping"), 0);
        }
        
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(GLuint)));
    }
};

Model loadUsingTinyObjLoader(std::string filename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	std::vector<float>* vertices = new std::vector<float>();
	std::vector<float>* normals = new std::vector<float>();
	std::vector<float>* textures = new std::vector<float>();
	std::vector<int>* indices = new std::vector<int>();

	// Loop over shapes
	int i = 0;
	for (size_t s = 0; s < shapes.size(); s++) {

		// Loop over faces (polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices->push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices->push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices->push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				normals->push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals->push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals->push_back(attrib.normals[3 * idx.normal_index + 2]);
				textures->push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				textures->push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				indices->push_back(i);
				i++;
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}

	float *vertexData = &(*vertices)[0];
	float *normalData =&(*normals)[0];
	float *textureData = &(*textures)[0];
	int *indexData = &(*indices)[0];

    Model model;
    model.vao = ModelUtils::createVAO(vertexData, vertices->size(),
                                  normalData, normals->size(),
                                  textureData, textures->size());
    model.indexBuffer = ModelUtils::createIndexBuffer(indexData, indices->size());
    model.numIndices = indices->size();

    delete vertices;
    delete normals;
    delete textures;
    delete indices;

    return model;   
}

namespace Box {
    float boxSize = 0.5f;

    float vertexCoordinates[] = {
        -boxSize,  boxSize,  boxSize,
        boxSize,  boxSize,  boxSize,
        boxSize, -boxSize,  boxSize,
        -boxSize, -boxSize,  boxSize,

        -boxSize,  boxSize,  -boxSize,
        boxSize,  boxSize,  -boxSize,
        boxSize, -boxSize,  -boxSize,
        -boxSize, -boxSize,  -boxSize,

        -boxSize,  boxSize,  -boxSize,
        -boxSize,  boxSize,  boxSize,
        -boxSize, -boxSize,  boxSize,
        -boxSize, -boxSize,  -boxSize,

        boxSize,  boxSize,  -boxSize,
        boxSize,  boxSize,  boxSize,
        boxSize, -boxSize,  boxSize,
        boxSize, -boxSize,  -boxSize,

        -boxSize,  boxSize,  -boxSize,
        boxSize,  boxSize,  -boxSize,
        boxSize, boxSize,  boxSize,
        -boxSize, boxSize,  boxSize,

        -boxSize,  -boxSize,  -boxSize,
        boxSize,  -boxSize,  -boxSize,
        boxSize, -boxSize,  boxSize,
        -boxSize, -boxSize,  boxSize,
    };

    float textureCoordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
    };

    float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,

        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
    };

    int indices[] {
        1, 0, 3,
        1, 3, 2,

        4, 5, 6,
        4, 6, 7,

        9, 8, 11,
        9, 11, 10,

        12, 13, 14,
        12, 14, 15,

        18, 17, 16,
        18, 16, 19,

        22, 23, 20,
        22, 20, 21
    };


    Model static createBox() {
        Model box;
        box.vao = ModelUtils::createVAO(vertexCoordinates, 3 * 24, normals, 3 * 24, textureCoordinates, 2 * 24);
        box.indexBuffer = ModelUtils::createIndexBuffer(indices, 36);
        box.textureId = loadPNGTexture("resources/gray.png");
        box.numIndices = 36;
        return box;
    }
};