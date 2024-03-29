#version 420

uniform isampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

out ivec4 result;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float step = float(1)/textureWidth;

    float minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    float maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

    ivec4 current = texture(texture1, texCoordInFS);
    int penetration = current.w;
    int offset = current.y;
    int contour = current.z;
    int numReceiving = current.x;

    int totalReceived = 0;
    for (int i = 0; i < offsetSize; i++) {
        vec2 newCoord = texCoordInFS + offsets[i] * step * 1;
        if (newCoord.x < 0 || newCoord.x > 1 || newCoord.y < 0 || newCoord.y > 1
            || newCoord.x < minTexCoordX || newCoord.x > maxTexCoordX || newCoord.y < minTexCoordY || newCoord.y > maxTexCoordY) {
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
    if (current.z == -3) { // Seed
        offset = offset + totalReceived;
    } else if (current.z == -2) { // obstacle
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

    result = ivec4(numReceiving, offset, contour, newPenetration);
}