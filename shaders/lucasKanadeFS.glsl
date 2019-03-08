#version 400

uniform sampler2D current;
uniform sampler2D dxTexture;
uniform sampler2D dyTexture;
uniform sampler2D dtTexture;

uniform int textureWidth;

in vec2 texCoordInFS;

out vec4 colorOut;

const int kernelWidth = 5;
const int kernelSize = kernelWidth * kernelWidth;

float w[kernelSize] = { 0.0030, 0.0133, 0.0219, 0.0133, 0.0030,
    0.0133, 0.0596, 0.0983, 0.0596, 0.0133,
    0.0219, 0.0983, 0.1621, 0.0983, 0.0219,
    0.0133, 0.0596, 0.0983, 0.0596, 0.0133,
    0.0030, 0.0133, 0.0219, 0.0133, 0.0030
};

float sumQ(float[kernelSize] q1, float[kernelSize] q2) {
    float sum = 0;
    for (int i = 0; i < kernelSize; i++) {
        sum += w[i] * q1[i] * q2[i];
    }
    return sum;
}

void main() {
    float h = float(1) / textureWidth;

    float dx[kernelSize];
    float dy[kernelSize];
    float dt[kernelSize];

    int halfSize = 2;// kernelWidth / 2;

    for (int x = -halfSize; x <= halfSize; x++) {
        for (int y = -halfSize; y <= halfSize; y++) {
            int i = (y + halfSize) * kernelWidth + (x + halfSize);

            dx[i] = texture(dxTexture, texCoordInFS + vec2(x * h, y * h)).r * 2 - 1;
            dy[i] = texture(dyTexture, texCoordInFS + vec2(x * h, y * h)).r * 2 - 1;
            dt[i] = texture(dtTexture, texCoordInFS + vec2(x * h, y * h)).r * 2 - 1;
        }
    }

    // (row, col)
    float m00 = sumQ(dx,dx);
    float m01 = sumQ(dx,dy);
    float m10 = sumQ(dx,dy);
    float m11 = sumQ(dy,dy);

    float n0 = -sumQ(dx,dt);
    float n1 = -sumQ(dy,dt);

    mat2 m = mat2(vec2(m00, m10), vec2(m10, m11));
    vec2 n = vec2(n0, n1);

    vec2 v = inverse(m) * n;

    // v = normalize((v));

    if (v.x < 0) {
        colorOut = vec4(1,0,0,1);
    } else {
        colorOut = vec4(0,1,0,1);
    }
    
    // colorOut = vec4(1, 1, 1, 1) * (v.x * 0.5 + 0.5) * 1;
    // colorOut = vec4(1, 1, 1, 1) * (v.y * 0.5 + 0.5) * 1;
    // colorOut = vec4(1, 1, 1, 1) * (v.y * 0.5 + 0.5);
    // colorOut = vec4(v.x, v.y, 0, 1) * 1;
    // colorOut = vec4(1, 1, 1, 1) * dx[0];
}