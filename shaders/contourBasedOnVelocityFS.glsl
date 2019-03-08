#version 400

/**
 * velocity map
 */
uniform sampler2D texture2;

/**
 * Penetration depth map.
 * Stores how much material each point should be displaced 
 *
 * Range: 0 indicates no penetration, -1 indicates maximum penetration.
 */
uniform sampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

uniform vec3 velocity;

in vec2 texCoordInFS;

out vec4 outNewDepth;

void main() {
    float step = float(1)/textureWidth;

    // Handle velocity near 0
    vec2 texvalue = texture(texture2, texCoordInFS).xy;

    vec2 velocityTex = normalize(texvalue);
    vec2 newCoord = texCoordInFS + velocityTex.xy * step;

    vec4 neighbour = abs(texture(texture1, newCoord));
    vec4 current =  abs(texture(texture1, texCoordInFS));

    float newDepth;
    if (current.b < 0.00001) { // Not part of penetration
        newDepth = (current.r);
    } else {
        newDepth = 0;
    }

    if (newCoord.x < 0 || newCoord.x > 1 || newCoord.y < 0 || newCoord.y > 1 || length(velocityTex.xy) < -0.1) {

    } else {
        if (neighbour.b > 0.00001 ) { // If neighbour is part of penetration
            newDepth += neighbour.r;
        }
    }
 
    outNewDepth = vec4(newDepth, 1, current.b, 1);
}