#version 400

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 projection;

out vec2 texCoordInFS;
out vec3 normalInFS;
out vec3 fragPosWorldSpaceInFS;
out vec3 cameraPosWorldSpaceInFS;

void main() {
    normalInFS = normal;
    vec4 worldCoordinate = modelToWorld * vec4(position, 1);
    fragPosWorldSpaceInFS = vec3(worldCoordinate.xyz);
    gl_Position = projection * worldToCamera * worldCoordinate;
    texCoordInFS = texCoord;

    mat4 worldToCameraInv = inverse(worldToCamera);
    cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);
}