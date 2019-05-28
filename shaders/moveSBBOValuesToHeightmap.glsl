#version 430

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer materialBuffer
{
    uint data[];
};

out uint newHeight;

vec2 texCoordToIntCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

int texCoordToSSBOIndex(vec2 texCoord) {
    vec2 intCoordinate = ivec2(texCoordToIntCoordinate(texCoord));
    return int(intCoordinate.y) * textureWidth + int(intCoordinate.x);
}

void main() {
    int index = texCoordToSSBOIndex(texCoordInFS);

    uint ssboValue = data[index];
    data[index] = 0;

    newHeight = ssboValue;
}