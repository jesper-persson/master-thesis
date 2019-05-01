#version 400

uniform usampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

// Scales the value read from the pixel to account for the fact that the
// width between two pixels is assumed to be 1.
uniform float heightScale;

uniform int heightColumnScale; 

in vec2 texCoordInFS;

out vec4 result;


void main() {
    float hx = 1/float(textureWidth);
    float hy = 1/float(textureHeight);

    float x0 = float(texture(texture1, texCoordInFS + vec2(-hx, 0)).r) / heightColumnScale;
    float x1 = float(texture(texture1, texCoordInFS + vec2(hx, 0)).r) / heightColumnScale;
    float y0 = float(texture(texture1, texCoordInFS + vec2(0, -hy)).r) / heightColumnScale;
    float y1 = float(texture(texture1, texCoordInFS + vec2(0, hy)).r) / heightColumnScale;

    // X normal normal
    float verticalDiffX = (x1 - x0) * 1 * heightScale;  
    float horizontalDiffX = 2;
    vec3 horDir = vec3(horizontalDiffX, 0, 0);
    vec3 verDir = vec3(0, verticalDiffX, 0);
    vec3 xNormal = horDir + verDir;

    // Y normal normal
    float verticalDiffY = (y1 - y0) * 1 * heightScale;
    float horizontalDiffY = 2;
    vec3 horDir2 = vec3(0, 0, horizontalDiffY);
    vec3 verDir2 = vec3(0, verticalDiffY, 0);
    vec3 yNormal = horDir2 + verDir2;

    vec3 normal = normalize(cross(yNormal, xNormal));
    result = vec4(normal.xyz, 1);
}

