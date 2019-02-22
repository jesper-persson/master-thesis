#version 400

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int textureSize;

in vec2 texCoordInFS;

out vec4 result;

float kernel[9] = float[] (0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625);

void main() {
    vec3 pixels[9];
    float step =  1 / float(textureSize);

    vec4 sum = vec4(0, 0, 0, 0);
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            sum += texture(texture1, texCoordInFS - vec2(step * i, step * j)) * kernel[(i+1)*3+(j+1)];
        }
    }

    vec3 sum2 = normalize(sum.xyz);
    result = vec4(sum2.xyz, 1);
}