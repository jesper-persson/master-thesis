#version 400

uniform usampler2D texture1; // heightmap
uniform sampler2D texture2; // obsticlemap
uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out uvec2 result;

const int heightColumnScale = 1000;
const int depthMapScale = 10;

void main() {
    uint v1 = texture(texture1, texCoordInFS).r;
    uint v2 = uint(texture(texture2, texCoordInFS).r * heightColumnScale * depthMapScale);
    result = uvec2(v1, v2);
}