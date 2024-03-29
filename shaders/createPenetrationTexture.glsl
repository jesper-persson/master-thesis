#version 400

uniform sampler2D texture1; // Depth texture
uniform usampler2D texture2; // Heightmap

in vec2 texCoordInFS;

uniform int textureWidth;

out ivec4 result;

uniform int heightColumnScale;
uniform int frustumHeight;

vec2 texCoordToIntCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float depthValue = texture(texture1, texCoordInFS).r;
    uint depthHeight = uint(depthValue * frustumHeight * heightColumnScale);

    uint heightmapValue = uint(texture(texture2, texCoordInFS).r);
    uint height = min(heightmapValue, depthHeight);
    int penetration = int(heightmapValue) - int(height);

    bool isPenetrating = penetration > 0;
    // depthValue > 0.00001 checks for first frame, where depthValue can be 0. Thus, it is assumed that
    // nothing penetrates the first frame.
    bool isObstacle = depthValue > 0.0001 && depthValue < 0.9999
        && (depthValue * heightColumnScale * frustumHeight - heightmapValue) <= 1000;

    if (isPenetrating && !isObstacle) {
        result = ivec4(0, 0, -1, 0);
    } 
    else if (isPenetrating && isObstacle) {
        result = ivec4(0, 0, -1, 0);
    }
    else if (!isPenetrating && isObstacle) {
        result = ivec4(0, 0, -2, 0); // the -2 indicates obstacle
    }
    else if (!isPenetrating && !isObstacle) {
        result = ivec4(texCoordToIntCoordinate(texCoordInFS).rg, -3, 0); // -3 indicates seed
    }
    result.w = penetration; 
}