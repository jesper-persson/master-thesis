#version 400

uniform isampler2D texture1;

in vec2 texCoordInFS;

out vec4 colorOutFS;

void main() {
    colorOutFS = vec4(1,1,1,1) * float(texture(texture1,texCoordInFS).w) / 1;
}