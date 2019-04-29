#version 430

// Heightmap
// uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

// new height
out uint outTexture;

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float step = float(1)/textureWidth;

    vec2 coordinate = texCoordToCoordinate(texCoordInFS);
    int index = int(coordinate.y) * textureWidth + int(coordinate.x);

    int ssboValue = data[index];
    data[index] = 0;

    // uint height = texture(texture1, texCoordInFS).r;

    outTexture = uint(ssboValue);
}