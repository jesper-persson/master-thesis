#version 400

uniform sampler2D texture1;
uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square
uniform int passIndex; // Goes from 0 to log2(textureWidth)

in vec2 texCoordInFS;

// (r, g) is a coordinate to closest "seed". And b is the manhattan distance.
// Initially (r, g) should be 0 for seeds. B should be 0 for seeds and -1 for non seeds.
out vec4 colorFS;

float manhattanDistance(vec2 d1, vec2 d2) {
    return abs(d1.x - d2.x) + abs(d1.y - d2.y);
    // return sqrt( pow(d1.x - d2.x,2) + pow(d1.y - d2.y, 2));
}

vec2 texCoordToCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

void main() {
    int offset =   int(pow(2, log2(textureWidth) - passIndex - 1));
    float step = float(1)/textureWidth;

    vec4 current = texture(texture1, texCoordInFS);
    if (current.b > -0.000001 && current.b < 0.000001) { // Discard if we are a seeds
        colorFS = current;
    } else {

        for (int y = -1; y < 2; y++) {
            for (int x = -1; x < 2; x++) {
                vec2 newTexCoord = texCoordInFS + vec2(step * offset * x, step * offset * y);
                vec4 neighbour = texture(texture1, newTexCoord);

                // neighbour.w  > 0.1 checks for obsticle
                if (neighbour.w > 0.1 || neighbour.z < -0.1 || newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y < 0 || newTexCoord.y > 1) {
                    continue;
                }

                float potentialClosest = manhattanDistance(texCoordToCoordinate(texCoordInFS), neighbour.xy);
                if (potentialClosest < current.z || current.z < -0.1) {
                    current.z = potentialClosest;
                    current.xy = neighbour.xy;
                }

            }
        }   
        colorFS = current;
    }
}