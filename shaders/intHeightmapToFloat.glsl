#version 400

uniform usampler2D texture1;
uniform int heightColumnScale;

in vec2 texCoordInFS;

out float height;

void main() {
    uint intHeight = texture(texture1, texCoordInFS).r;
    height = intHeight / float(heightColumnScale);
}