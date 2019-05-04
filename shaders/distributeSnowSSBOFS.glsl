#version 430

uniform isampler2D texture1; // Distance texture

uniform usampler2D texture2; // Heightmap texture

uniform int textureWidth;
uniform int textureHeight;

uniform float compression;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer snowBuffer
{
    uint data[];
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

int intCoordinateToSSBOIndex(vec2 intCoordinate) {
    return int(intCoordinate.y) * textureWidth + int(intCoordinate.x);
} 

void main() {
    // Read how much snow this pixel should move

    // vec4 tex1Value = texture(texture1, texCoordInFS);

    // Move to SSBO location (given by my coordinates to closest )
    ivec4 cloestSeedI = texture(texture1, texCoordInFS).rgba;
    uint penetration = uint(cloestSeedI.w); //texture(texture2, texCoordInFS).r;
    vec2 cloestSeed = vec2(float(cloestSeedI.x), float(cloestSeedI.y));
    vec2 myCoord = texCoordToCoordinate(texCoordInFS);
    vec2 dirClosest = cloestSeed - myCoord;
    float lengthToCloest = length(dirClosest);// * length(dirClosest);
    // cloestSeed = cloestSeed + normalize(dirClosest) * lengthToCloest;

    int indexCloest = int(cloestSeed.y) * textureWidth + int(cloestSeed.x);
    int indexMe = int(myCoord.y) * textureWidth + int(myCoord.x);

    uint heightmapValue = texture(texture2, texCoordInFS).r;

    int numDivisions = int(penetration) / 5000;
    penetration = penetration / numDivisions;
    atomicAdd(data[indexMe], heightmapValue - penetration * numDivisions );
    for (int i = 0; i < numDivisions; i++) {
        vec2 target = cloestSeed + normalize(dirClosest) * (length(dirClosest) + i);
        int targetIndex = intCoordinateToSSBOIndex(target);
        atomicAdd(data[targetIndex], uint(penetration * (1 - compression)));
    }
}