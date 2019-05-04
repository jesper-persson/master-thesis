#version 420

uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square texture

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

uniform float terrainSize;
uniform float slopeThreshold;
uniform float roughness;

uniform int heightColumnScale;
uniform int frustumHeight;

in vec2 texCoordInFS;

out uvec4 result;
const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    float d = terrainSize/float(textureWidth);
    return atan(h2 - h1) / d;
}

void main() {
    float minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    float maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

    float step = float(1)/textureWidth;

    uvec4 currentTex = texture(texture1, texCoordInFS);
    uint currentHeight = currentTex.r;
    uint obstacleCurrent = currentTex.g;

    uint avgHeightDiff = 0;
    uint numWithTooHighSlope = 0;

    uvec4 textureValues[offsetSize];
    textureValues[0] = textureOffset(texture1, texCoordInFS, ivec2(-1, 1));
    textureValues[1] = textureOffset(texture1, texCoordInFS, ivec2(0, 1));
    textureValues[2] = textureOffset(texture1, texCoordInFS, ivec2(1, 1));
    textureValues[3] = textureOffset(texture1, texCoordInFS, ivec2(-1, 0));
    textureValues[4] = textureOffset(texture1, texCoordInFS, ivec2(1, 0));
    textureValues[5] = textureOffset(texture1, texCoordInFS, ivec2(-1, -1));
    textureValues[6] = textureOffset(texture1, texCoordInFS, ivec2(0, -1));
    textureValues[7] = textureOffset(texture1, texCoordInFS, ivec2(1, -1));

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        uvec4 neighbourTex =  textureValues[i];
        uint obstacle = neighbourTex.g;
        uint h = neighbourTex.r;

        bool canReceiveMaterial = obstacle - h > 1000 || obstacle > (float(frustumHeight) - 0.1) * heightColumnScale;
        if (!canReceiveMaterial
            || newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0
            || newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY) {
            continue;
        }

        float s = slope(h / float(heightColumnScale), currentHeight / float(heightColumnScale));

        if (s > slopeThreshold) {
            avgHeightDiff += currentHeight - h;
            numWithTooHighSlope += 1;
        }
    }

    bool canTransferMaterial = obstacleCurrent - currentHeight > 1000 || obstacleCurrent > (float(frustumHeight) - 0.1) * heightColumnScale;
    
    uint totalToRemove = 0;
    uint neighbourQuota = 0;

    if (numWithTooHighSlope > 0  && canTransferMaterial) {
        avgHeightDiff /= numWithTooHighSlope;
        avgHeightDiff = uint(avgHeightDiff * roughness);

        totalToRemove = avgHeightDiff - avgHeightDiff % numWithTooHighSlope;
        neighbourQuota = avgHeightDiff / numWithTooHighSlope;
    }

    result = uvec4(totalToRemove, neighbourQuota, currentHeight, obstacleCurrent);
}