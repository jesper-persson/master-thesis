#version 420

/**
 * r: num neighbours with less contour value
 * g: -
 * b: contour value
 * a: penetration value
 */
uniform isampler2D texture1; // Result from calc num rec

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

/**
 * r: new penetration depth [-1,0]
 * g: new penetration depth [-1,0]
 * b: contour value
 * a: num neighbours with less contour value
 */
out ivec4 outNewDepth;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float step = float(1)/textureWidth;

    ivec4 current = texture(texture1, texCoordInFS);
    int penetration = current.w;
    int offset = current.y;
    int contour = current.z;
    int numReceiving = current.x;

    int totalReceived = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newCoord = texCoordInFS + offsets[i] * step * 1;
        if (newCoord.x < 0 || newCoord.x > 1 || newCoord.y < 0 || newCoord.y > 1) {
            continue;
        }

        ivec4 currentInner = texture(texture1, newCoord);
        int penetrationInner = currentInner.w;
        int contourInner = currentInner.z;
        int numReceivingInner = currentInner.x;
        if (numReceivingInner > 0 && contourInner > contour && contourInner > -1) {
            totalReceived += (penetrationInner) / numReceivingInner;
        }
    }
    
    int newPenetration = 0;
    
    
    // Giving
        // newPenetration = totalReceived;
        
    // if (current.z == -3) { // Seed
    //     newPenetration = penetration + totalReceived;
    //     offset = newPenetration;
    // } else if (current.z == -2) { // Obsticle
    //     newPenetration = penetration;
    //     offset = offset;
    // } else {

    //     newPenetration = penetration - totalReceived;
    //     if (numReceiving > 0) {
    //         newPenetration = newPenetration + abs(penetration) - abs(penetration) % numReceiving;
    //     }

    //     offset = offset + totalReceived;
    //     if (numReceiving > 0) {
    //         offset = offset - abs(penetration) + abs(penetration) % numReceiving;
    //     }
    // }

    //outNewDepth = ivec4(newPenetration, offset, contour, numReceiving);

    if (current.z == -3) { // Seed
        // newPenetration = penetration + totalReceived;
        offset = offset + totalReceived;
    } else if (current.z == -2) { // Obsticle
        newPenetration = penetration;
        offset = offset;
    } else {

        newPenetration = penetration + totalReceived;
        if (numReceiving > 0) {
            newPenetration = newPenetration - penetration + penetration % numReceiving;
        }

        offset = offset + totalReceived;
        if (numReceiving > 0) {
            offset = offset - penetration + penetration % numReceiving;
        }
    }

    outNewDepth = ivec4(numReceiving, offset, contour, newPenetration);
}