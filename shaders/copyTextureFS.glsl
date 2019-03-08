#version 400

uniform sampler2D texture1;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    result = texture(texture1, texCoordInFS);
}