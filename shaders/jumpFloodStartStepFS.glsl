#version 400

/*
 * This shader produces the precondition needed for jumpFloodFS
 */

uniform sampler2D texture1; // Penetration texture (0 for non penetration, negative otherwise)
uniform sampler2D texture2; // Prev penetratin texture [0, -1]
uniform int textureWidth;

in vec2 texCoordInFS;

// (r,g) is coordinate to closest 0. B is 0 if we dont displace. 
// alpha is wheter we are an obsticle
out vec4 colorFS;

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    float penetration = abs(texture(texture1, texCoordInFS).r);
    float prevPenetration = abs(texture(texture2, texCoordInFS).r);

    bool isPenetrating = penetration > 0.0001;
    bool wasPenetrating = prevPenetration < 0.99; // < 0.1 check for initial black texture

    if (isPenetrating && !wasPenetrating) {
        colorFS = vec4(0, 0, -1, 0);
    } 
    else if (wasPenetrating) {
        colorFS = vec4(0, 0, 0, 1); // the 1 indicates obsticle
    }
    else if (!isPenetrating && !wasPenetrating) {
        colorFS = vec4(texCoordToCoordinate(texCoordInFS).rg, 0, 0);
    }
}