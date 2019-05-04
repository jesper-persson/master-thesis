#version 420

/**
 * This shader is used by the iterative algorithm for moving penetrated material to the boundary.
 * Each column needs to know how many neighbors will receive its material, since we cannot "scatter" material.
 */

// Contains result of JumpFlood shader program.
uniform isampler2D texture1;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

in vec2 texCoordInFS;

uniform float compression;

out ivec4 outputValue;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    float maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

    ivec4 current = texture(texture1, texCoordInFS);
    float step = float(1)/textureWidth;
    int penetration = current.w;

    int numNeighboursWithLessDistance = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0 || 
            newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY
            ) {
            continue;
        }
        vec4 currentInner = texture(texture1, newTexCoord);
        if (currentInner.z < current.z && currentInner.z != -2) { // -2 checks for obstacle
            numNeighboursWithLessDistance++;
        }
    }

    int offset = -int(penetration * compression);
    penetration = int((1 - compression) * penetration);

    if (current.z < -1) {
        numNeighboursWithLessDistance = 0;
    }

    outputValue = ivec4(numNeighboursWithLessDistance, offset, current.z, penetration);
}