#version 400

uniform sampler2D texture1; // Depth texture [0,1]
uniform usampler2D texture2; // Heightmap

in vec2 texCoordInFS;

out int result;

const int heightColumnScale = 1000;

void main() {
    float verticalScale = 10; // Should be uniform
    float depthValue = texture(texture1, texCoordInFS).r;
    uint depthHeight = uint(depthValue * verticalScale * heightColumnScale);

    uint heightmapValue = uint(texture(texture2, texCoordInFS).r);
    // float heightmapValue = texture(texture2, texCoordInFS).r;
    uint height = min(heightmapValue, depthHeight);
    // if (depthHeight > 9.9) {
    //     height = heightmapValue;
    // }
    // result = int(height) - int(heightmapValue);
    result = int(heightmapValue) - int(height);
}

// a-b = -(b-a)