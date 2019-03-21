#version 400

/**
 * r: penetration depth
 * g: penetration depth
 * b: contour value
 * a: num neighbours with less contour value
 */
uniform sampler2D texture1; // between 0 and -1

uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 outNewDepth;

const int offsetSize = 8;
// vec2 offsets[offsetSize] = {vec2(0, 1), vec2(-1, 0), vec2(1, 0), vec2(0, -1)};
vec2 offsets[offsetSize] = {vec2(-1, 1), vec2(0, 1), vec2(1, 1), vec2(-1, 0), vec2(1, 0), vec2(-1, -1), vec2(0, -1), vec2(1, -1)};

void main() {
    float step = float(1)/textureWidth;

    vec4 val = texture(texture1, texCoordInFS);
    float c0 = val.b;
    float numRec = val.a;
    float d0 = val.r;
    float contours[offsetSize];
    float depth[offsetSize];

    // float iteration = val.b + 1;

    for (int i = 0; i < offsetSize; i++) {
        vec2 newCoord = texCoordInFS + offsets[i] * step * 1;
        vec4 cTexture = texture(texture1, newCoord);
        float c1 = cTexture.b;
        float numReceiving = cTexture.a;
        float d1 = 0;
        if (numReceiving > 0.1) {
            d1 = abs(cTexture.r) / float(numReceiving);
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

    outNewDepth = vec4(newDepth, newDepth, c0, numRec);
}