#version 400

uniform sampler2D texture1;
uniform int textureWidth;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float hx = 1/float(textureWidth);

    vec3 x0 = texture(texture1, texCoordInFS + vec2(-hx, 0)).xyz;
    vec3 x1 = texture(texture1, texCoordInFS + vec2(0, 0)).xyz;
    vec3 x2 = texture(texture1, texCoordInFS + vec2(hx, 0)).xyz;

    float x0f = x0.x;// + x0.y + x0.z;
    float x2f = x2.x;// + x2.y + x2.z;

    float derivative = (-x2f + x0f) * 0.5 + 0.5;

    result = vec4(1, 1, 1, 1) * derivative;
}
