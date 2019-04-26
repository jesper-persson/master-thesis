#version 420

/**
//  * (r, g, b): height value
//  * a: obsticle value
 */
uniform usampler2D texture1; // uint heihtmap


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

const int heightColumnScale = 1000;


in vec2 texCoordInFS;

/**
 * r: the amount of material this pixel should remove from its height,
 * g: how much material each "valid" neighbour should receive,
 * b: the current height,
 * a: obsticle value 
 */
out uvec4 colorFS;

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
    uint obsticleFragmentCurrent = currentTex.g ;

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
        uint obsticleFragment = neighbourTex.g ;
        uint h = neighbourTex.r;

        bool canReceiveSnow = obsticleFragment - h > 1000 || obsticleFragment > 9.9 * heightColumnScale;
        if (!canReceiveSnow) {
            continue;
        }

        // if (obsticleFragment - h < 1 * heightColumnScale && obsticleFragment <= 9.99 * heightColumnScale ) {
        //     continue;
        // }

        // if (obsticleFragment - h <= 1) {
        //     continue;
        // }

        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }
        if (newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY) {
            continue;
        }

        // float s = slope(currentHeight / float(heightColumnScale), h / float(heightColumnScale));
        float s = slope(h / float(heightColumnScale), currentHeight / float(heightColumnScale));

        if (s > slopeThreshold) {
            avgHeightDiff += currentHeight - h;
            numWithTooHighSlope += 1;
        }
    }

    uint totalToRemove = 0;
    uint neighbourQuota = 0;

    bool canTransferSnow = obsticleFragmentCurrent - currentHeight > 1000 || obsticleFragmentCurrent > 9.9 * heightColumnScale;

    if (numWithTooHighSlope > 0.1  && canTransferSnow) { // && (!(obsticleFragmentCurrent - currentHeight < 1 * heightColumnScale) || obsticleFragmentCurrent > 9.99 * heightColumnScale))  {
        avgHeightDiff /= numWithTooHighSlope;
        avgHeightDiff = uint(avgHeightDiff * roughness);

        totalToRemove = avgHeightDiff - avgHeightDiff % numWithTooHighSlope;
        neighbourQuota = avgHeightDiff / numWithTooHighSlope; // Take remainder and add to total to remove
    }

    colorFS = uvec4(totalToRemove, neighbourQuota, currentHeight, obsticleFragmentCurrent);
    // colorFS = uvec4(totalToRemove, neighbourQuota, numWithTooHighSlope, obsticleFragmentCurrent);
}