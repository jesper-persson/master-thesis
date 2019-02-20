#version 400

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 projection;

out vec2 textCoordOutVs;
out vec3 normalOutVs;
out vec3 fragPosWorldSpaceOutVs;

void main() {
    normalOutVs = normal;
    vec4 worldCoordinate = modelToWorld * vec4(position, 1);
    fragPosWorldSpaceOutVs = vec3(worldCoordinate.xyz);
    gl_Position = projection * worldToCamera * worldCoordinate;
    textCoordOutVs = texCoord;
}