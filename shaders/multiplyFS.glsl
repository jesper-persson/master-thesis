#version 400

uniform sampler2D texture1;
uniform float factor;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    vec4 color1 = texture(texture1, texCoordInFS);
    result = color1 * factor;
}