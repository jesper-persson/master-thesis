#version 430

// Heightmap
// uniform sampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

/**
 * r: height
 * g: height
 * b: height
 * a: height
 */
out vec4 outTexture;

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float step = float(1)/textureWidth;

    vec2 coordinate = texCoordToCoordinate(texCoordInFS);
    int index = int(coordinate.y) * textureWidth + int(coordinate.x);

    float ssboValue = data[index] / 10000.0;
    if (ssboValue > 0.00001 || ssboValue < -0.000001) {
        outTexture = vec4(1,1,1,1) * ssboValue;
    } else {
        outTexture = vec4(0,0,0,0);
    }
    data[index] = 0;
}