#version 400

uniform sampler2D texture1; // Depth texture [0,1]
uniform usampler2D texture2; // Heightmap

in vec2 texCoordInFS;

uniform int textureWidth;

out ivec4 result;

uniform int heightColumnScale;
uniform int frustumHeight;


vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float depthValue = texture(texture1, texCoordInFS).r;
    uint depthHeight = uint(depthValue * frustumHeight * heightColumnScale);

    uint heightmapValue = uint(texture(texture2, texCoordInFS).r);
    // float heightmapValue = texture(texture2, texCoordInFS).r;
    uint height = min(heightmapValue, depthHeight);
    // if (depthHeight > 9.9) {
    //     height = heightmapValue;
    // }
    // result = int(height) - int(heightmapValue);
    int penetration = int(heightmapValue) - int(height);
    float prevPenetration = depthValue;

    // Added from jumpflood start step

    bool isPenetrating = penetration > 0;
    // prevPenetration > 0.00001 checks for first frame, where prevPenetration can be 0. Thus, it is assumed that
    // nothing penetrates the first frame.
    bool wasPenetrating = prevPenetration > 0.0001 && prevPenetration < 0.9999;

    if (isPenetrating && !wasPenetrating) {
        result = ivec4(0, 0, -1, 0);
    } 
    else if (isPenetrating && wasPenetrating/* && penetration < prevPenetration*/) { // if we come from above
        result = ivec4(0, 0, -1, 0);
    }
    else if (!isPenetrating && wasPenetrating) {
        result = ivec4(0, 0, -2, 0); // the -2 indicates obsticle
    }
    else if (!isPenetrating && !wasPenetrating) {
        result = ivec4(texCoordToCoordinate(texCoordInFS).rg, -3, 0);
    }
    result.w = penetration; 
}

// a-b = -(b-a)