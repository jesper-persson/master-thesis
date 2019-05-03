#version 400

uniform sampler2D texture1;

in vec2 texCoordInFS;

out uint result;

uniform int heightColumnScale;
uniform int frustumHeight;

void main() {
    result = uint(texture(texture1, texCoordInFS).r * frustumHeight * heightColumnScale);
}