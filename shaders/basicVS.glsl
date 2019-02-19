#version 400

layout(location=0) in vec3 position;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 projection;

void main() {
    gl_Position = projection * worldToCamera * modelToWorld * vec4(position, 1);
}