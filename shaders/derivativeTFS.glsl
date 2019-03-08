#version 400

uniform sampler2D texture1;
uniform sampler2D texture2;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float x0 = texture(texture1, texCoordInFS + vec2(0, 0)).x;
    float x1 = texture(texture2, texCoordInFS + vec2(0, 0)).x;

    float derivative = (-x1 + x0) * 0.5 + 0.5;

    result = vec4(1, 1, 1, 1) * derivative;
}

