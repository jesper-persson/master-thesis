#version 420

/**
 * r: the amount of material this pixel should remove from its height,
 * g: how much material each "valid" neighbour should receive,
 * b: the current height,
 * a: obsticle value 
 */
uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform float terrainSize;
uniform float slopeThreshold;

in vec2 texCoordInFS;

/**
 * (r, g, b): height
 * a: obsticle
 */
out uvec2 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

float slope(float h1, float h2) {
    float d = terrainSize / float(textureWidth);
    return atan(h2*1 - h1*1) / d;
}

const int heightColumnScale = 1000;

void main() {
    float step = float(1)/textureWidth;
    uvec4 currentTex = texture(texture1, texCoordInFS);
    uint currentH = currentTex.b;
    uint toRemove = currentTex.r;
    uint currentObsticleFragment = currentTex.a;

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

    bool canReceiveSnow = currentObsticleFragment - currentH > 1000 || currentObsticleFragment > 9.9 * heightColumnScale;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;

        uvec4 neighbourTex = textureValues[i];
        uint h = neighbourTex.b;
        uint obsticleFragment = neighbourTex.a;

        bool canTransferSnow = obsticleFragment - h > 1000 || obsticleFragment > 9.9 * heightColumnScale;

        // if (((neighbourTex.a - h) < 0.01 && neighbourTex.a <= 9.99 * heightColumnScale) || (currentTex.a  - currentH < 0.01 && currentTex.a <= 9.99 * heightColumnScale)) {
        //     continue;
        // }

        // if (obsticleFragment - h <= 1) {
        //     continue;
        // }

        if (!canTransferSnow) {
            continue;
        }

        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float slope = slope(currentH / float(heightColumnScale), h / float(heightColumnScale));

        if (slope > slopeThreshold && canReceiveSnow) {
            avgHeight += neighbourTex.g;
        }
    }

    colorFS = uvec2(avgHeight, currentTex.a);
    // colorFS = uvec4(avgHeight, avgHeight, numRec, avgHeight);
}