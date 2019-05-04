#version 430

uniform sampler2D texture1; // obstacle map

uniform int textureWidth;
uniform int textureHeight;

uniform float terrainSize;
uniform float roughness;
uniform float slopeThreshold;

in vec2 texCoordInFS;

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

uniform int heightColumnScale;
uniform int frustumHeight;

layout(std430, binding = 2) buffer materialBuffer
{
    uint data[];
};

vec2 texCoordToIntCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

vec2 SSBOIndexToTexCoord(int index) {
    return vec2(index % textureWidth, index / textureWidth) / float(textureWidth);
}

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2 - h1) / d;
}

float minTexCoordX;
float maxTexCoordX;
float minTexCoordY;
float maxTexCoordY;

bool withinBounds(int index) {
    vec2 newTexCoord = SSBOIndexToTexCoord(index);
    return index >= 0 && index <= (textureWidth * textureWidth - 1) && 
           !(newTexCoord.x < minTexCoordX || newTexCoord.x >= maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y >= maxTexCoordY);
}

bool canReceiveMaterial(float obstacleValue, uint height) {
    float obstacleFragment = obstacleValue * frustumHeight * heightColumnScale;
    return obstacleFragment - height > 1000 || obstacleFragment > (float(frustumHeight) - 0.1) * heightColumnScale;
}

const int numNeighbors = 8;

void main() {
    vec2 coordinate = texCoordToIntCoordinate(texCoordInFS);
    int index = int(coordinate.y) * textureWidth + int(coordinate.x);
    uint ssboValue = data[index];

    minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

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

    uint ssboValues[numNeighbors];
    float obstacleValue[numNeighbors];
    for (int i = 0; i < numNeighbors; i++) {
        if (withinBounds(indexMap[i])) {
            ssboValues[i] = data[indexMap[i]];
            obstacleValue[i] = texture(texture1, SSBOIndexToTexCoord(indexMap[i])).r;
        }
    }

    uint avgHeightDiff = 0;
    uint numNeighborsWithTooGreatSlope = 0;
    for (int i = 0; i < numNeighbors; i++) {
        if (!withinBounds(indexMap[i]) || !canReceiveMaterial(obstacleValue[i], ssboValues[i])) {
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
        avgHeightDiff = uint(avgHeightDiff * roughness);
        uint numHeightToGivePerNeighbor = avgHeightDiff / numNeighborsWithTooGreatSlope;

        atomicAdd(data[index], -numNeighborsWithTooGreatSlope * numHeightToGivePerNeighbor);
        for (int i = 0; i < numNeighbors; i++) {
            if (!withinBounds(indexMap[i]) || !canReceiveMaterial(obstacleValue[i], ssboValues[i])) {
                continue;
            }
            float slopeValue = slope(ssboValues[i]/float(heightColumnScale), ssboValue/float(heightColumnScale) );
            if (slopeValue > slopeThreshold) {
                atomicAdd(data[indexMap[i]], numHeightToGivePerNeighbor);
            }
        }
    }
}