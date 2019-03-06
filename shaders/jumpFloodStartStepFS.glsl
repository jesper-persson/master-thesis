#version 400

/*
 * This shader produces the precondition needed for jumpFloodFS
 */

uniform sampler2D texture1; // Penetration texture (0 for non penetration, non-zero otherwise)
uniform sampler2D texture2;
uniform int textureWidth;

in vec2 texCoordInFS;

out vec4 colorFS;

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float penetration = texture(texture1, texCoordInFS).r;

    if (penetration > 0.0001) {
        colorFS = vec4(0, 0, -1, 1);
        // colorFS = vec4(1, 0, 0, 1);
    } else {
        colorFS = vec4(texCoordToCoordinate(texCoordInFS).rg, 0, 1);
        // colorFS = vec4(1, 1, 1, 1);
    }

    // colorFS = vec4(penetration,penetration,penetration, 1);
}