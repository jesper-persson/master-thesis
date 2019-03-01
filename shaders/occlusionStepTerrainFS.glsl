#version 400

uniform sampler2D texture1;
uniform sampler2D normalMap;
uniform sampler2D ssaoMap;
uniform sampler2D noiseSSAO;
uniform sampler2D normalMapMacro;
uniform int normalMapRepeat;
uniform mat4 projection;

uniform mat4 worldToCamera;

in mat4 invTransposeWorldToCamera;
in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;
in vec3 fragPosViewSpaceInFS;
in vec3 cameraPosWorldSpaceInFS;
in mat3 TBNInFs;

out vec4 colorOutFs;

// Needed for SSAO texture lookup
float WINDOW_HEIGHT = 1200;
float WINDOW_WIDTH = 1800;

const int kernelSize = 64;
uniform vec3 kernel[kernelSize];

vec2 texCoordNoiseScale = vec2(WINDOW_WIDTH / 4.0, WINDOW_HEIGHT / 4.0);

float depthValueForViewSpaceCoord(vec3 viewSpaceCoord) {
    vec4 coord = vec4(viewSpaceCoord, 1);
    coord = projection * coord;
    coord.xyz /= coord.w;
    coord.xy = coord.xy * 0.5 + 0.5;
    return texture(ssaoMap, coord.xy).r;
}

float toViewSpaceDepth(float depth) {
    vec4 viewspace = inverse(projection) * vec4(0, 0, 2.0 * depth - 1.0,  1.0);
    return viewspace.z / viewspace.w;
}

float calculateOcclusion(vec3 normal) {
    float occlusion = 0;
    float radius = 1;

    vec3 normalViewSpace = normalize((invTransposeWorldToCamera * vec4(normalize(normal), 1)).xyz);

    vec4 coord = projection * vec4(fragPosViewSpaceInFS, 1);
    coord.xyz /= coord.w;
    coord.xy = coord.xy * 0.5 + 0.5;

    vec3 randomVec = (texture(noiseSSAO, texCoordInFS * texCoordNoiseScale).rgb);
    randomVec = normalize(texture(noiseSSAO, coord.xy * texCoordNoiseScale).rgb);
    vec3 tangent   = normalize(randomVec - normalViewSpace * dot(randomVec, normalViewSpace));
    vec3 bitangent = normalize(cross(normalViewSpace, tangent));
    mat3 TBN       = mat3(tangent, bitangent, normalViewSpace);  
    
    float numInsideGeometry = 0;
    for (int i = 0; i < kernelSize; i++) {
        vec3 ssample = TBN *  kernel[i];
        ssample = fragPosViewSpaceInFS + ssample * radius;

        float depthValue = depthValueForViewSpaceCoord(ssample);
        float viewSpaceDepthValue = toViewSpaceDepth(depthValue);

        float limitRange = smoothstep(0.0, 1.0, radius / abs(fragPosViewSpaceInFS.z - ssample.z));
        if (ssample.z > viewSpaceDepthValue + 0.025) {
            numInsideGeometry = numInsideGeometry + 1  * limitRange;
        }
    }

    float occlusionfaktor = 1.0 - numInsideGeometry / float(kernelSize);
    return occlusionfaktor;
}

void main() {
    vec3 normalBase = texture(normalMapMacro, texCoordInFS * vec2(1, -1)).rgb;
    vec3 normal = normalize(normalBase);

    // SSAO
    float occlusionFactor = calculateOcclusion(normal);
    colorOutFs = vec4(1, 1, 1, 1) * (1 - occlusionFactor);
}