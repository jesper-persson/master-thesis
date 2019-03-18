#version 400

/**
 * This shader implements a part of the erosion step of the erosion shader of "Animating Sand, Mud, and snow".
 */ 

// Height map
uniform sampler2D texture1;

// Obsticle map. Contains obsticles so that ground does not erode into rigid bodies
uniform sampler2D texture2;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform int activeWidth; // Allows to simulate part of texture
uniform int activeHeight; // Allows to simulate part of texture
uniform int activeCenterX;
uniform int activeCenterY;

uniform float terrainSize;
uniform float slopeThreshold;

in vec2 texCoordInFS;

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
    float roughness = 0.2;

    float heights[offsetSize];
    float slopes[offsetSize];

    float currentH = texture(texture1, texCoordInFS).r;
    float obsticleFragmentCurrent = texture(texture2, texCoordInFS).r;

    float avgHeightDiff = 0;
    float numWithTooHighSlope = 0;

    float slopeSum = 0;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        float obsticleFragment = texture(texture2, newTexCoord).r;
        float h = texture(texture1, newTexCoord).r;

        if (obsticleFragment - h < 1 && obsticleFragment <= 9.99 ) {
            continue;
        }
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }
        if (newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY) {
            continue;
        }

        heights[i] = h;
        slopes[i] = slope(currentH, h);

        if (slopes[i] > slopeThreshold) {
            avgHeightDiff += currentH - h;
            numWithTooHighSlope += 1.0;
            slopeSum += slopes[i];
        }
    }

    float totalToRemove = 0;
    float neighbourQuota = 0;

    if (numWithTooHighSlope > 0.1 && (!(obsticleFragmentCurrent - currentH < 1) || obsticleFragmentCurrent > 9.99))  {
        avgHeightDiff /= (numWithTooHighSlope);
        avgHeightDiff *= roughness;
        totalToRemove = avgHeightDiff;
        neighbourQuota = totalToRemove / numWithTooHighSlope;
    }

    colorFS = vec4(totalToRemove,neighbourQuota,currentH,obsticleFragmentCurrent);
}