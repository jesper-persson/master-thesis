#version 400

/**
 * This shader implements the erosion step of "Animating Sand, Mud, and snow".
 */ 

// output from calc avg height
uniform sampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform float terrainSize;
uniform float slopeThreshold;

in vec2 texCoordInFS;

/**
 * (r, g, b): height
 * a: obsticle
 */
out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

void main() {
    float step = float(1)/textureWidth;
    vec4 currentTex = texture(texture1, texCoordInFS);
    float currentH = currentTex.b;
    float toRemove = currentTex.r;

    float avgHeight = currentH - toRemove;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;

        vec4 neighbourTex = texture(texture1, texCoordInFS + offsets[i] * step);
        float h = neighbourTex.b;

        if (((neighbourTex.a - h) < 0.01 && neighbourTex.a <= 9.99) || (currentTex.a - currentH < 0.01 && currentTex.a <= 9.99)) {
            continue;
        }

        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float slope = slope(h, currentH);

        if (slope > slopeThreshold) {
            float totToRemove = neighbourTex.r;
            avgHeight += neighbourTex.g;
        }
    }

    colorFS = vec4(avgHeight, avgHeight, avgHeight, currentTex.a);
}