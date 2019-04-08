#version 420

// Contains result of JumpFlood shader program.
uniform sampler2D texture1;

// Penetration depth map.
uniform sampler2D texture2;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

uniform float compressionRatio;

in vec2 texCoordInFS;

/**
 * r: penetration value
 * g: penetration value
 * b: contourValue (distance to closest target)
 * a: num neighbours with less contour value
 */
out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    vec4 currentContour = texture(texture1, texCoordInFS);
    float penetrationValue = texture(texture2, texCoordInFS).r;
    float step = float(1)/textureWidth;

    float numNeighboursWithLessDistance = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        vec4 contour = texture(texture1, newTexCoord);
        if (contour.b < currentContour.b && contour.w < 0.1) {
            numNeighboursWithLessDistance++;
        }
    }

    if (currentContour.w > 0.1) {
        currentContour.b = 100000;
    }

    penetrationValue = penetrationValue * compressionRatio;

    colorFS = vec4(penetrationValue, penetrationValue, currentContour.b, numNeighboursWithLessDistance);
}