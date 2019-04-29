#version 430

uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

void main() {
    vec2 coordinate = texCoordToCoordinate(texCoordInFS);
    int index = int(coordinate.y) * textureWidth + int(coordinate.x);
    int ssboValue = data[index];

    int ssboValues[numNeighbors];
    ssboValues[0] = data[index - textureWidth - 1];
    ssboValues[1] = data[index - textureWidth];
    ssboValues[2] = data[index - textureWidth + 1];
    ssboValues[3] = data[index - 1];
    ssboValues[4] = data[index + 1];
    ssboValues[5] = data[index + textureWidth - 1];
    ssboValues[6] = data[index + textureWidth];
    ssboValues[7] = data[index + textureWidth + 1];

    const int numNeighbors = 8;
    int indexMap[numNeighbors] = {
        index - textureWidth - 1,
        index - textureWidth,
        index - textureWidth + 1,
        index - 1,
        index + 1,
        index + textureWidth - 1,
        index + textureWidth,
        index + textureWidth + 1
    };


    int avgHeightDiff = 0;
    int numNeighborsWithTooGreatSlope = 0;
    for (int i = 0; i < numNeighbors; i++) {
        float slopeValue = slope(float(ssboValue), float(ssboValues[i]));
        if (slopeValue > slopeThreshold) {
            avgHeightDiff += abs(ssboValue - ssboValues[i]);
            numNeighborsWithTooGreatSlope++; 
        }
    }

    int numHeightToGive = 0;
    if (numNeighborsWithTooGreatSlope > 0) {
        numHeightToGive = avgHeightDiff / numNeighborsWithTooGreatSlope;
        atomicAdd(data[index], numNeighborsWithTooGreatSlope * numHeightToGive);
    }
    for (int i = 0; i < numNeighbors; i++) {
        float slopeValue = slope(float(ssboValue), float(ssboValues[i]));
        if (slopeValue > slopeThreshold) {
            atomicAdd(data[indexMap[i]], numHeightToGive);
        }
    }
}