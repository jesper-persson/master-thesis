#version 400

/**
 * This shader implements the erosion step of "Animating Sand, Mud, and snow".
 */ 

// height map
uniform sampler2D texture1;

// avg height diff
uniform sampler2D texture2;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform int activeWidth; // Allows to simulate part of texture
uniform int activeHeight; // Allows to simulate part of texture

uniform float terrainSize;
uniform float slopeThreshold;

in vec2 texCoordInFS;

out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    float d = terrainSize/float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

void main() {
    float step = float(1)/textureWidth;
    float currentH = texture(texture1, texCoordInFS).r;
    vec4 data = texture(texture2, texCoordInFS);
    float toRemove = data.r;

    float avgHeight = currentH - toRemove;
    // int numWithTooHighSlope = 0;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;

        vec4 avgTex = texture(texture2, texCoordInFS + offsets[i] * step);
        float h = avgTex.b;

        if (((avgTex.a - h) < 0.01 && avgTex.a <= 9.99) || (data.a - currentH < 0.01 && data.a <= 9.99)) {
            continue;
        }

        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float slope = slope(h, currentH);

        if (slope > slopeThreshold) {
            float totToRemove = avgTex.r;
            // float slopeSum = avgTex.b;
            avgHeight += avgTex.g;
            // if (slopeSum > 0) {
            //     avgHeight += slope/slopeSum * totToRemove;
            // }
        }
    }

    colorFS = vec4(1, 1, 1, 1) * avgHeight;
}