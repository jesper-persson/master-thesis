#version 430

/*
 * This shader produces the precondition needed for jumpFloodFS
 */

uniform isampler2D texture1; // Penetration texture (0 for non penetration, negative otherwise)
uniform sampler2D texture2; // Prev penetratin texture [0, -1]
uniform int textureWidth;

in vec2 texCoordInFS;

// (r,g) is coordinate to closest 0. B is 0 if we dont displace. 
// alpha is wheter we are an obsticle
out ivec4 colorFS;

// layout(std430, binding = 2) buffer snowBuffer
// {
//     int data[];
// };


vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    // vec2 myCoord = texCoordToCoordinate(texCoordInFS);
    // int index = int(myCoord.y) * textureWidth + int(myCoord.x);
    // data[index] = 0;

    int penetration = texture(texture1, texCoordInFS).r;
    float prevPenetration = (texture(texture2, texCoordInFS).r);

    bool isPenetrating = penetration > 0;
    // prevPenetration > 0.00001 checks for first frame, where prevPenetration can be 0. Thus, it is assumed that
    // nothing penetrates the first frame.
    // bool wasPenetrating = prevPenetration < 9.99999 && prevPenetration > 0.00001;
    bool wasPenetrating = prevPenetration > 0.0001 && prevPenetration < 0.9999;

    if (isPenetrating && !wasPenetrating) {
        colorFS = ivec4(0, 0, -1, 0);
    } 
    else if (isPenetrating && wasPenetrating/* && penetration < prevPenetration*/) { // if we come from above
        colorFS = ivec4(0, 0, -1, 0);
    }
    else if (!isPenetrating && wasPenetrating) {
        colorFS = ivec4(0, 0, -2, 0); // the -2 indicates obsticle
    }
    else if (!isPenetrating && !wasPenetrating) {
        colorFS = ivec4(texCoordToCoordinate(texCoordInFS).rg, -3, 0);
    }
    colorFS.w = penetration; 

    // if (isPenetrating && !wasPenetrating) {
    //     colorFS = vec4(0, 0, -1, 0);
    // } 
    // else if (isPenetrating && wasPenetrating && penetration < prevPenetration) { // if we come from above
    //     colorFS = vec4(0, 0, -1, 0);
    // }
    // else if (wasPenetrating) {
    //     colorFS = vec4(0, 0, 0, 1); // the 1 indicates obsticle
    // }
    // else if (!isPenetrating && !wasPenetrating) {
    //     colorFS = vec4(texCoordToCoordinate(texCoordInFS).rg, 0, 0);
    // }
}