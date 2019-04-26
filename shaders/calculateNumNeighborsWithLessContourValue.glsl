#version 420

/**
 * This shader is used by the iterative algorithm for moving penetrated snow to the boundary.
 * Each column needs to know how many neighbors will receive its snow, since we cannot "scatter" snow.
 */

// Contains result of JumpFlood shader program.
uniform isampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

in vec2 texCoordInFS;

/**
 * r: num neighbours with less contour value
 * g: -
 * b: contour value
 * a: penetration value
 */
out ivec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    ivec4 current = texture(texture1, texCoordInFS);
    int penetrationValue = current.w;
    float step = float(1)/textureWidth;

    int numNeighboursWithLessDistance = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }
        vec4 currentInner = texture(texture1, newTexCoord);
        if (currentInner.z < current.z && currentInner.z != -2) { // -2 checks for obsticle
            numNeighboursWithLessDistance++;
        }
    }

    if (current.z < -1) {
        numNeighboursWithLessDistance = 0;   
    }

    colorFS = ivec4(penetrationValue, 0, current.z, numNeighboursWithLessDistance);
}