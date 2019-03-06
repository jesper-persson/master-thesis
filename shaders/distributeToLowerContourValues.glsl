#version 400

/**
 * Contour map (distance field)
 */
uniform sampler2D texture2;

/**
 * Penetration depth map.
 * Stores how much material each point should be displaced 
 */
uniform sampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 outNewDepth;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float step = float(1)/textureWidth;

    vec4 val = texture(texture2, texCoordInFS);
    float c0 = val.r;
    float numRec = val.g;
    float d0 = texture(texture1, texCoordInFS).r;
    float contours[offsetSize];
    float depth[offsetSize];

    for (int i = 0; i < offsetSize; i++) {
        vec2 newCoord = texCoordInFS + offsets[i] * step;
        vec4 cTexture = texture(texture2, newCoord);
        float c1 = cTexture.r;
        float numReceiving = cTexture.g;
        float d1 = 0;
        if (numReceiving > 0.1) {
            d1 = texture(texture1, newCoord).r / float(numReceiving);
        }
        if (newCoord.x < 0 || newCoord.x > 1 || newCoord.y < 0 || newCoord.y > 1) {
            c1 = -1;
            d1 = 0;
        }
        contours[i] = c1;            
        depth[i] = d1;            
    }

    float newDepth = d0;
    if (c0 < 0.00001) { // First outer ring
        newDepth = d0;
    } else {
        newDepth = 0;
    }
    
    for (int i = 0; i < offsetSize; i++) {
        if (contours[i] > c0) {
            newDepth += depth[i];
        }
    }

    if (c0 > 0.0001 && numRec < 0.1) {
        newDepth = 0;
    }

    outNewDepth = vec4(1, 1, 1, 1) * newDepth;
}