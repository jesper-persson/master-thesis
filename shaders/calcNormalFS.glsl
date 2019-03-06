#version 400

uniform sampler2D texture1;
uniform sampler2D texture2; // This is a unary operation, so it doesn't need a second texture
uniform int textureWidth;
uniform int textureHeight;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    float hx = 1/float(textureWidth);
    float hy = 1/float(textureHeight);

    float x0 = texture(texture1, texCoordInFS + vec2(-hx, 0)).r;
    float x1 = texture(texture1, texCoordInFS + vec2(hx, 0)).r;
    float y0 = texture(texture1, texCoordInFS + vec2(0, -hy)).r;
    float y1 = texture(texture1, texCoordInFS + vec2(0, hy)).r;

    // 30 corresponds to with of terrain
    float pixelWidthX = 30/float(textureWidth) * 1; // With of pixel in worldspace
    float pixelWidthY = 30/float(textureHeight) * 1; // With of pixel in worldspace

    // X normal normal
    float verticalDiffX = (x1 - x0) * 10;
    float horizontalDiffX = pixelWidthX * 1;
    vec3 horDir = vec3(horizontalDiffX, 0, 0);
    vec3 verDir = vec3(0, verticalDiffX, 0);
    vec3 xNormal = horDir + verDir;

    // Y normal normal
    float verticalDiffY = y1 - y0;
    float horizontalDiffY = pixelWidthY * 1;
    vec3 horDir2 = vec3(0, 0, horizontalDiffY);
    vec3 verDir2 = vec3(0, verticalDiffY, 0);
    vec3 yNormal = horDir2 + verDir2;

    vec3 normal = normalize(cross(yNormal, xNormal));
    result = vec4(normal.xyz, 1);
}

