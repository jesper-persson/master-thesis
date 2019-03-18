#version 400

uniform sampler2D texture1;

uniform int textureWidth;
uniform int textureHeight;

// Scales the value read from the pixel to account for the fact that the
// with between two pixels is assumed to be 1.
uniform float heightScale;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float hx = 1/float(textureWidth);
    float hy = 1/float(textureHeight);

    float x0 = texture(texture1, texCoordInFS + vec2(-hx, 0)).r;
    float x1 = texture(texture1, texCoordInFS + vec2(hx, 0)).r;
    float y0 = texture(texture1, texCoordInFS + vec2(0, -hy)).r;
    float y1 = texture(texture1, texCoordInFS + vec2(0, hy)).r;

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

