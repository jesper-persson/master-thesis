#version 400

uniform sampler2D texture1; // Flood
uniform sampler2D texture2;
uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

in vec2 texCoordInFS;

out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float currentContour = texture(texture1, texCoordInFS).b;
    float step = float(1)/textureWidth;

    float numNeighboursWithLessDistance = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        float contour = texture(texture1, newTexCoord).b;
        if (contour < currentContour) {
            numNeighboursWithLessDistance++;
        }
    }

    colorFS = vec4(currentContour, numNeighboursWithLessDistance, 1, 1);
}