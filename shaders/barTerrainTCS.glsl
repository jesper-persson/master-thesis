#version 400

layout (vertices = 3) out;

uniform sampler2D heightmap;

uniform mat4 worldToCamera;
uniform mat4 projection;

uniform int heightColumnScale;

in vec3 fragPosWorldSpaceInTCS[];
in vec2 texCoordInTCS[];
in vec3 normalInTCS[];

out vec3 fragPosWorldSpaceInTES[];
out vec2 texCoordInTES[];
out vec3 normalInTES[];
out float sampledHeightInTES[];

void main() {
    texCoordInTES[gl_InvocationID] = texCoordInTCS[gl_InvocationID];
    normalInTES[gl_InvocationID] = normalInTCS[gl_InvocationID];
    fragPosWorldSpaceInTES[gl_InvocationID] = fragPosWorldSpaceInTCS[gl_InvocationID];

    float[3] heights;
    heights[0] = texture(heightmap, texCoordInTCS[0]).x;
    heights[1] = texture(heightmap, texCoordInTCS[1]).x;
    heights[2] = texture(heightmap, texCoordInTCS[2]).x;
    float max = -1;
    for (int i = 0; i < 3; i++) {
        if (heights[i] > max) {
            max = heights[i];
        }
    }
    bool allSame = true;
    for (int i = 0; i < 3; i++) {
        if (heights[i] != max) {
            allSame = false;
        }
    }
    if (allSame) {
        max*=1.1;
    }
    sampledHeightInTES[0] = max;
    sampledHeightInTES[1] = max;
    sampledHeightInTES[2] = max;


    gl_TessLevelOuter[0] = 1;
    gl_TessLevelOuter[1] = 1;
    gl_TessLevelOuter[2] = 1;
    gl_TessLevelInner[0] = 1;
}