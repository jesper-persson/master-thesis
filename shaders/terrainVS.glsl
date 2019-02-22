#version 400

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;

uniform mat4 modelToWorld;

out vec3 fragPosWorldSpaceInTCS;
out vec2 texCoordInTCS;
out vec3 normalInTCS;

void main()
{
    fragPosWorldSpaceInTCS = (modelToWorld * vec4(position, 1.0)).xyz;
    texCoordInTCS = texCoord;
    normalInTCS = (modelToWorld * vec4(normal, 0.0)).xyz;
}