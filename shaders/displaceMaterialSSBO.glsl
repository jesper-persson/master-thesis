#version 430

uniform isampler2D texture1; // Distance texture
uniform usampler2D texture2; // Heightmap texture

uniform int textureWidth;
uniform int textureHeight;

uniform float compression;

const bool useMultipleTargets = false;

in vec2 texCoordInFS;

layout(std430, binding = 2) buffer materialBuffer
{
    uint data[];
};

vec2 texCoordToIntCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

int intCoordinateToSSBOIndex(vec2 intCoordinate) {
    return int(intCoordinate.y) * textureWidth + int(intCoordinate.x);
} 

void main() {
    vec2 currentIntCoordinate = texCoordToIntCoordinate(texCoordInFS);
    int currentSSBOIndex = intCoordinateToSSBOIndex(currentIntCoordinate);
    
    uint heightmapValue = texture(texture2, texCoordInFS).r;
    
    ivec4 distanceTextureValue = texture(texture1, texCoordInFS).rgba;
    uint penetration = uint(distanceTextureValue.w);
    vec2 cloestSeedIntCoordinate = vec2(float(distanceTextureValue.x), float(distanceTextureValue.y));
    vec2 delta = cloestSeedIntCoordinate - currentIntCoordinate;
    vec2 dirToClosestSeed = normalize(delta);
    float lengthToClosestSeed = length(delta);

    if (useMultipleTargets) {
        int numDivisions = int(ceil(int(penetration) / 5000.0));
        penetration = penetration / numDivisions;
        atomicAdd(data[currentSSBOIndex], heightmapValue - penetration * numDivisions);
        for (int i = 0; i < numDivisions; i++) {
            vec2 target = cloestSeedIntCoordinate + dirToClosestSeed * (lengthToClosestSeed + i);
            int targetSSBOIndex = intCoordinateToSSBOIndex(target);
            atomicAdd(data[targetSSBOIndex], uint(penetration * (1 - compression)));
        }
    } else {
        atomicAdd(data[currentSSBOIndex], heightmapValue - penetration);
        int targetSSBOIndex = intCoordinateToSSBOIndex(cloestSeedIntCoordinate);
        atomicAdd(data[targetSSBOIndex], uint(penetration * (1 - compression)));
    }

}