#version 430

/**
 * r: coordinate to cloest seed
 * g: coordinate to cloest seed
 * b: ?
 * a: ?
 */
uniform sampler2D texture1;

/**
 * Penetration texture.
 */
uniform sampler2D texture2;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer snowBuffer
{
    int data[];
};

/**
 * r: ?
 * g: ?
 * b: ?
 * a: ?
 */
out vec4 outNewDepth;

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    // Read how much snow this pixel should move
    float penetration = texture(texture2, texCoordInFS).r;

    // Move to SSBO location (given by my coordinates to closest )
    vec2 cloestSeed = texture(texture1, texCoordInFS).rg;
    vec2 myCoord = texCoordToCoordinate(texCoordInFS);
    vec2 dirClosest = cloestSeed - myCoord;
    float lengthToCloest = length(dirClosest);
    cloestSeed = cloestSeed + normalize(dirClosest) * lengthToCloest;

    int indexCloest = int(cloestSeed.y) * textureWidth + int(cloestSeed.x);
    int indexMe = int(myCoord.y) * textureWidth + int(myCoord.x);

    // Move my penetration to cloest seed
    data[indexMe] += int(penetration * 10000);
    //  data[indexCloest] += -int(penetration * 10000);

    atomicAdd(data[indexCloest], -int(penetration * 10000));

    // data[int(cloestSeed.y) * textureWidth + int(cloestSeed.x)] = penetration;


}