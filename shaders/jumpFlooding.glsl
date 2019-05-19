#version 400

uniform isampler2D texture1;
uniform int textureWidth;
uniform int textureHeight; // Not used, since we assume square
uniform int passIndex;

in vec2 texCoordInFS;

// Allows to simulate part of texture
uniform int activeWidth;
uniform int activeHeight;
uniform int activeCenterX;
uniform int activeCenterY;

out ivec4 result;

float distanceMetric(vec2 d1, ivec2 d2) {
    return max(abs(d1.x - d2.x), abs(d1.y - d2.y));
    // return abs(d1.x - d2.x) + abs(d1.y - d2.y);
    //return sqrt( pow(d1.x - d2.x,2) + pow(d1.y - d2.y, 2));
}

vec2 texCoordToIntCoordinate(vec2 texCoord) {
    return texCoord * textureWidth;
}

vec2 intCoordinateToTexCoord(vec2 intCoord) {
    return intCoord / float(textureWidth);
}

const int distanceMetricScale = 1000;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 neighbors[8];

void main() {
    int offset =  int(pow(2, log2(textureWidth) - passIndex - 1));
    float step = float(1)/textureWidth;

    neighbors[0] = texCoordInFS + vec2(step * offset * -1, step * offset * 1);
    neighbors[1] = texCoordInFS + vec2(step * offset * 0, step * offset * 1);
    neighbors[2] = texCoordInFS + vec2(step * offset * 1, step * offset * 1);
    neighbors[3] = texCoordInFS + vec2(step * offset * -1, step * offset * 0);
    neighbors[4] = texCoordInFS + vec2(step * offset * 1, step * offset * 0);
    neighbors[5] = texCoordInFS + vec2(step * offset * -1, step * offset * -1);
    neighbors[6] = texCoordInFS + vec2(step * offset * 0, step * offset * -1);
    neighbors[7] = texCoordInFS + vec2(step * offset * 1, step * offset * -1);

    float minTexCoordX = (activeCenterX - activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float maxTexCoordX = (activeCenterX + activeWidth/float(2) + textureWidth/float(2))/float(textureWidth); 
    float minTexCoordY = (activeCenterY - activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 
    float maxTexCoordY = (activeCenterY + activeHeight/float(2) + textureHeight/float(2))/float(textureHeight); 

    ivec4 current = texture(texture1, texCoordInFS);
    if (current.b == -2 || current.b == -3) { // Discard if we are a seeds
        result = current;
    } else {
        
        int randomStart = int(ceil(rand(texCoordInFS) * 7.0));
        for (int j = 0; j < 8; j++) {
            randomStart++;
            int index = (randomStart) % 8;
            vec2 newTexCoord = neighbors[index];
            ivec4 neighbour = texture(texture1, newTexCoord);
            vec2 pointTexCoord = intCoordinateToTexCoord(neighbour.xy);

            // neighbour.b == -2 checks for obstilce
            if (neighbour.b == -2 || newTexCoord.x < 0 || newTexCoord.x > 1 || newTexCoord.y < 0 || newTexCoord.y > 1
                || newTexCoord.x < minTexCoordX || newTexCoord.x > maxTexCoordX || newTexCoord.y < minTexCoordY || newTexCoord.y > maxTexCoordY
                || pointTexCoord.x < minTexCoordX || pointTexCoord.x > maxTexCoordX || pointTexCoord.y < minTexCoordY || pointTexCoord.y > maxTexCoordY) {
                continue;
            }

            int potentialClosest = int(distanceMetricScale * distanceMetric(texCoordToIntCoordinate(texCoordInFS), neighbour.xy));
            if (potentialClosest < current.z || current.z < 0) {
                current.z = potentialClosest;
                current.xy = neighbour.xy;
            }
        
        }
        result = current;
    }
}