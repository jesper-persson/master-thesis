#version 420

uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform float terrainSize;
uniform float slopeThreshold;

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

uniform int heightColumnScale;
uniform int frustumHeight;

in vec2 texCoordInFS;

out uvec2 result;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

void main() {
    float minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    float maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

    float step = float(1)/textureWidth;
    uvec4 currentTex = texture(texture1, texCoordInFS);
    uint currentH = currentTex.b;
    uint toRemove = currentTex.r;
    uint obsticleCurrent = currentTex.a;

    uint avgHeight = currentH - toRemove;

    uvec4 textureValues[offsetSize];
    textureValues[0] = textureOffset(texture1, texCoordInFS, ivec2(-1, 1));
    textureValues[1] = textureOffset(texture1, texCoordInFS, ivec2(0, 1));
    textureValues[2] = textureOffset(texture1, texCoordInFS, ivec2(1, 1));
    textureValues[3] = textureOffset(texture1, texCoordInFS, ivec2(-1, 0));
    textureValues[4] = textureOffset(texture1, texCoordInFS, ivec2(1, 0));
    textureValues[5] = textureOffset(texture1, texCoordInFS, ivec2(-1, -1));
    textureValues[6] = textureOffset(texture1, texCoordInFS, ivec2(0, -1));
    textureValues[7] = textureOffset(texture1, texCoordInFS, ivec2(1, -1));

    bool canReceiveSnow = obsticleCurrent - currentH > 1000 || obsticleCurrent > (float(frustumHeight) - 0.1) * heightColumnScale;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;        
        uvec4 neighbourTex = textureValues[i];
        uint h = neighbourTex.b;
        uint obsticleFragment = neighbourTex.a;
        bool canTransferSnow = obsticleFragment - h > 1000 || obsticleFragment > (float(frustumHeight) - 0.1) * heightColumnScale;

        if (newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY
            || !canTransferSnow
            || newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float slope = slope(currentH / float(heightColumnScale), h / float(heightColumnScale));

        if (slope > slopeThreshold && canReceiveSnow) {
            avgHeight += neighbourTex.g;
        }
    }

    result = uvec2(avgHeight, currentTex.a);
}