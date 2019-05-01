#version 420

uniform isampler2D texture1; // Offset map
uniform usampler2D texture2; // Heightmap
uniform sampler2D texture3; // Obstacle map

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

in vec2 texCoordInFS;

out uvec2 theOutput;

uniform int heightColumnScale;
uniform int frustumHeight;

void main() {
    ivec4 offset = texture(texture1, texCoordInFS);
    uint currentHeightmap = texture(texture2, texCoordInFS).r;
    uint newHeightmap = uint(int(currentHeightmap) + offset.y);
    
    uint obstacle = uint(texture(texture3, texCoordInFS).r * heightColumnScale * frustumHeight);
    theOutput = uvec2(newHeightmap, obstacle);
}