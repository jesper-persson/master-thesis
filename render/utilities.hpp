#pragma once

#include "../glm/mat4x4.hpp"

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