#version 430

uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

uniform float terrainSize;
uniform float roughness;
uniform float slopeThreshold;

in vec2 texCoordInFS;

const int heightColumnScale = 1000;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2 - h1) / d;
}

bool withinBounds(int index) {
    return index >= 0 && index <= (textureWidth * textureWidth - 1);
}

const int numNeighbors = 8;

void main() {
    vec2 coordinate = texCoordToCoordinate(texCoordInFS);
    int index = int(coordinate.y) * textureWidth + int(coordinate.x);
    int ssboValue = data[index];

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

    int ssboValues[numNeighbors];
    for (int i = 0; i < numNeighbors; i++) {
        if (withinBounds(indexMap[i])) {
            ssboValues[i] = data[indexMap[i]];
        }
    }

    int avgHeightDiff = 0;
    int numNeighborsWithTooGreatSlope = 0;
    for (int i = 0; i < numNeighbors; i++) {
        if (!withinBounds(indexMap[i])) {
            continue;
        }
        float slopeValue = slope(ssboValue/float(heightColumnScale), ssboValues[i]/float(heightColumnScale));
        if (slopeValue < slopeThreshold) {
            avgHeightDiff += (ssboValue - ssboValues[i]);
            numNeighborsWithTooGreatSlope++;
        }
    }

    int numHeightToGive = 0;
    if (numNeighborsWithTooGreatSlope > 0) {
        numHeightToGive =  avgHeightDiff / numNeighborsWithTooGreatSlope;
        numHeightToGive = int(numHeightToGive * roughness);

        atomicAdd(data[index], -numNeighborsWithTooGreatSlope * numHeightToGive);
        for (int i = 0; i < numNeighbors; i++) {
            if (!withinBounds(indexMap[i])) {
                continue;
            }
            float slopeValue = slope(ssboValue/float(heightColumnScale), ssboValues[i]/float(heightColumnScale));
            if (slopeValue < slopeThreshold) {
                atomicAdd(data[indexMap[i]], numHeightToGive);
            }
        }
    }
}