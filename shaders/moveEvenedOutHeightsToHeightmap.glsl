#version 400

uniform usampler2D texture1;

in vec2 texCoordInFS;

out uint result;

void main() {
    result = texture(texture1, texCoordInFS).r;
}