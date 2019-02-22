#version 400

layout (vertices = 3) out;

in vec3 fragPosWorldSpaceInTCS[];
in vec2 texCoordInTCS[];
in vec3 normalInTCS[];

out vec3 fragPosWorldSpaceInTES[];
out vec2 texCoordInTES[];
out vec3 normalInTES[];

void main()
{
    texCoordInTES[gl_InvocationID] = texCoordInTCS[gl_InvocationID];
    normalInTES[gl_InvocationID] = normalInTCS[gl_InvocationID];
    fragPosWorldSpaceInTES[gl_InvocationID] = fragPosWorldSpaceInTCS[gl_InvocationID];

    int TL = 50;

    gl_TessLevelOuter[0] = TL;
    gl_TessLevelOuter[1] = TL;
    gl_TessLevelOuter[2] = TL;
    gl_TessLevelInner[0] = TL;
}