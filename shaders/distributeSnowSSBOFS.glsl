#version 430

/**
 * r: coordinate to cloest seed
 * g: coordinate to cloest seed
 * b: ?
 * a: ?
 */
uniform isampler2D texture1;

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

    // vec4 tex1Value = texture(texture1, texCoordInFS);

    // Move to SSBO location (given by my coordinates to closest )
    ivec4 cloestSeedI = texture(texture1, texCoordInFS).rgba;
    int penetration = cloestSeedI.w; //texture(texture2, texCoordInFS).r;
    vec2 cloestSeed = vec2(float(cloestSeedI.x), float(cloestSeedI.y));
    vec2 myCoord = texCoordToCoordinate(texCoordInFS);
    vec2 dirClosest = cloestSeed - myCoord;
    float lengthToCloest = length(dirClosest);// * length(dirClosest);
    // cloestSeed = cloestSeed + normalize(dirClosest) * lengthToCloest;

    int indexCloest = int(cloestSeed.y) * textureWidth + int(cloestSeed.x);
    int indexMe = int(myCoord.y) * textureWidth + int(myCoord.x);

    // Move my penetration to cloest seed
    //  data[indexCloest] += -int(penetration * 10000);

    // data[indexMe] += int(penetration * 10000);

    float compression = 1;

    // if (texCoordInFS.x > 0.25 && texCoordInFS.x < 0.65 && texCoordInFS.y > 0.25 && texCoordInFS.y < 0.75) {
        atomicAdd(data[indexMe], int(penetration));
        atomicAdd(data[indexCloest], -int(penetration * compression));

        // data[indexMe] = 1 * 100000;
    // }

    // data[int(cloestSeed.y) * textureWidth + int(cloestSeed.x)] = penetration;


}