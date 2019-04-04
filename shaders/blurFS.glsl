#version 400

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 result;

float kernel[9] = float[] (0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625);

void main() {
    vec3 pixels[9];
    float stepX =  1 / float(textureWidth);
    float stepY =  1 / float(textureHeight);

    vec4 sum = vec4(0, 0, 0, 0);
    sum += textureOffset(texture1, texCoordInFS, ivec2(-1, 1)) * kernel[0];
    sum += textureOffset(texture1, texCoordInFS, ivec2(0, 1)) * kernel[1];
    sum += textureOffset(texture1, texCoordInFS, ivec2(1, 1)) * kernel[2];
    sum += textureOffset(texture1, texCoordInFS, ivec2(-1, 0)) * kernel[3];
    sum += textureOffset(texture1, texCoordInFS, ivec2(0, 0)) * kernel[4];
    sum += textureOffset(texture1, texCoordInFS, ivec2(1, 0)) * kernel[5];
    sum += textureOffset(texture1, texCoordInFS, ivec2(-1, -1)) * kernel[6];
    sum += textureOffset(texture1, texCoordInFS, ivec2(0, -1)) * kernel[7];
    sum += textureOffset(texture1, texCoordInFS, ivec2(1, -1)) * kernel[8];

    vec3 sum2 = sum.xyz;
    result = vec4(sum2.xyz, 1);
}