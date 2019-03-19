#version 400

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float v1 = texture(texture1, texCoordInFS).r;
    float v2 = texture(texture2, texCoordInFS).r;
    result = vec4(v1, v1, v1, v2);
}