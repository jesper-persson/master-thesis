#version 400

uniform sampler2D texture1; // Flood
uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square

in vec2 texCoordInFS;

out vec4 colorFS;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    vec4 currentContour = texture(texture1, texCoordInFS);
    float step = float(1)/textureWidth;

    float numNeighboursWithLessDistance = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newTexCoord = texCoordInFS + offsets[i] * step;
        if (newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y > 1 || newTexCoord.y < 0) {
            continue;
        }

        vec4 contour = texture(texture1, newTexCoord);
        if (contour.b < currentContour.b && contour.w < 0.1) {
            numNeighboursWithLessDistance++;
        }
    }

    if (currentContour.w > 0.1) {
        currentContour.b = 100000;
    }

    colorFS = vec4(currentContour.b, numNeighboursWithLessDistance, 0, 1);
}