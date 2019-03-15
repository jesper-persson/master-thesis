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

in vec2 texCoordInFS;

out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    /**
     * Remember to change d in "erosionFS.glsl" as well (should be uniform)
     *
     * d is the length between two columns in world coords (length of a column).
     */
    float terrainSize = 30;
    float d = terrainSize/float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

void main() {
    float step = float(1)/textureWidth;
    float slopeThreshold = 0.09; // Change ing calcAvgHeight shader too
   
    float currentH = texture(texture1, texCoordInFS).r;
    vec4 data = texture(texture2, texCoordInFS);
    float toRemove = data.r;

    float avgHeight = currentH - toRemove;
    // int numWithTooHighSlope = 0;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;

        vec4 avgTex = texture(texture2, texCoordInFS + offsets[i] * step);

        if (avgTex.a < 0.99 || data.a < 0.99) {
            continue;
        }

        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float h = avgTex.b; //texture(texture1, newTexCoord).r;
        float slope = slope(h, currentH);

        if (slope > slopeThreshold) {
            vec4 texVal = texture(texture2, texCoordInFS + offsets[i] * step);
            float totToRemove = texVal.r;
            // float slopeSum = texVal.b;
            avgHeight += avgTex.g;
            // if (slopeSum > 0) {
            //     avgHeight += slope/slopeSum * totToRemove;
            // }
        }
    }

    colorFS = vec4(1, 1, 1, 1) * avgHeight;
}