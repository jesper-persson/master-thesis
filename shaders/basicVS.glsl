#version 400

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 projection;

out vec2 texCoordInFS;
out vec3 normalInFS;
out vec3 fragPosWorldSpaceInFS;
out vec3 fragPosViewSpaceInFS;
out vec3 cameraPosWorldSpaceInFS;
out mat4 invTransposeWorldToCamera;

out mat3 TBNInFs;

void main() {
    normalInFS = normalize(inverse(transpose(modelToWorld)) * vec4(normal, 1)).xyz;
    vec4 worldCoordinate = modelToWorld * vec4(position, 1);
    fragPosWorldSpaceInFS = vec3(worldCoordinate.xyz);
    fragPosViewSpaceInFS = vec3(worldToCamera * worldCoordinate);
    gl_Position = projection * worldToCamera * worldCoordinate;
    texCoordInFS = texCoord;

    // Build TBN matrix
    vec3 T = normalize(vec3(modelToWorld * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(modelToWorld * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(modelToWorld * vec4(normal,  0.0)));
    TBNInFs = mat3(T, B, N);

    mat4 worldToCameraInv = inverse(worldToCamera);
    cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);

    invTransposeWorldToCamera = inverse(transpose(worldToCamera));
}