#version 430

uniform sampler2D texture1; // Obsticle map

uniform int textureWidth;
uniform int textureHeight;

uniform float terrainSize;
uniform float roughness;
uniform float slopeThreshold;

in vec2 texCoordInFS;

uniform int heightColumnScale;
uniform int frustumHeight;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

vec2 SSBOIndexToTexCoord(int index) {
    return vec2(index % textureWidth, index / textureWidth) / float(textureWidth);
}

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2 - h1) / d;
}

bool withinBounds(int index) {
    return index >= 0 && index <= (textureWidth * textureWidth - 1);
}

bool canReceiveSnow(float obsticleValue, int height) {
    float obsticleFragment = obsticleValue * frustumHeight * heightColumnScale;
    return obsticleFragment - height > 1000 || obsticleFragment > (float(frustumHeight) - 0.1) * heightColumnScale;
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
    float obstacleValue[numNeighbors];
    for (int i = 0; i < numNeighbors; i++) {
        if (withinBounds(indexMap[i])) {
            ssboValues[i] = data[indexMap[i]];
            obstacleValue[i] = texture(texture1, SSBOIndexToTexCoord(indexMap[i])).r;
        }
    }

    int avgHeightDiff = 0;
    int numNeighborsWithTooGreatSlope = 0;
    for (int i = 0; i < numNeighbors; i++) {
        if (!withinBounds(indexMap[i])) {
            continue;
        }
        if (!canReceiveSnow(obstacleValue[i], ssboValues[i])) {
            continue;
        }
        float slopeValue = slope(ssboValues[i]/float(heightColumnScale), ssboValue/float(heightColumnScale) );
        if (slopeValue > slopeThreshold) {
            avgHeightDiff += (ssboValue - ssboValues[i]);
            numNeighborsWithTooGreatSlope++;
        }
    }

    if (numNeighborsWithTooGreatSlope > 0) {
        avgHeightDiff = avgHeightDiff / numNeighborsWithTooGreatSlope;
        avgHeightDiff = int(avgHeightDiff * roughness);
        int numHeightToGivePerNeighbor = avgHeightDiff / numNeighborsWithTooGreatSlope;

        atomicAdd(data[index], -numNeighborsWithTooGreatSlope * numHeightToGivePerNeighbor);
        for (int i = 0; i < numNeighbors; i++) {
            if (!withinBounds(indexMap[i])) {
                continue;
            }
            if (!canReceiveSnow(obstacleValue[i], ssboValues[i])) {
                continue;
            }
        float slopeValue = slope(ssboValues[i]/float(heightColumnScale), ssboValue/float(heightColumnScale) );
        if (slopeValue > slopeThreshold) {
                atomicAdd(data[indexMap[i]], numHeightToGivePerNeighbor);
            }
        }
    }
}