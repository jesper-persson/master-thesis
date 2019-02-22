#version 400

uniform sampler2D texture1;

in vec2 texCoordInFS;

out vec4 colorOutFS;

void main() {
    vec4 color = texture(texture1, texCoordInFS);
    colorOutFS = color;
}