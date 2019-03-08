#version 400

uniform sampler2D texture1;
uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float v = texture(texture1, texCoordInFS).r;
    result = vec4(1, 1, 1, 1) * v;
}