#version 420

uniform isampler2D texture1;
uniform usampler2D texture2;

uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

in vec2 texCoordInFS;

out uint newHeightmap;

void main() {
    ivec4 offset = texture(texture1, texCoordInFS);
    uint currentHeightmap = texture(texture2, texCoordInFS).r;

    newHeightmap = uint(int(currentHeightmap) + offset.y);
}