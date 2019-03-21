#version 400

/**
 * This shader implements a part of the erosion step of the erosion shader of "Animating Sand, Mud, and snow".
 */ 

/**
 * (r, g, b): height value
 * a: obsticle value
 */
uniform sampler2D texture1;

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

in vec2 texCoordInFS;

/**
 * r: the amount of material this pixel should remove from its height,
 * g: how much material each "valid" neighbour should receive,
 * b: the current height,
 * a: obsticle value 
 */
out vec4 colorFS;

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

    vec4 currentTex = texture(texture1, texCoordInFS);
    float currentHeight = currentTex.r;
    float obsticleFragmentCurrent = currentTex.a;

    float avgHeightDiff = 0;
    float numWithTooHighSlope = 0;
    float slopeSum = 0;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        vec4 neighbourTex = texture(texture1, newTexCoord);
        float obsticleFragment = neighbourTex.a;
        float h = neighbourTex.r;

        if (obsticleFragment - h < 1 && obsticleFragment <= 9.99 ) {
            continue;
        }
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }
        if (newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY) {
            continue;
        }

        float s = slope(currentHeight, h);

        if (s > slopeThreshold) {
            avgHeightDiff += currentHeight - h;
            numWithTooHighSlope += 1.0;
            slopeSum += s;
        }
    }

    float totalToRemove = 0;
    float neighbourQuota = 0;

    if (numWithTooHighSlope > 0.1 && (!(obsticleFragmentCurrent - currentHeight < 1) || obsticleFragmentCurrent > 9.99))  {
        avgHeightDiff /= numWithTooHighSlope;
        avgHeightDiff *= roughness;
        totalToRemove = avgHeightDiff;
        neighbourQuota = totalToRemove / numWithTooHighSlope;
    }

    colorFS = vec4(totalToRemove, neighbourQuota, currentHeight, obsticleFragmentCurrent);
}