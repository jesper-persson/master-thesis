#version 400

layout (vertices = 3) out;

uniform sampler2D heightmap;

in vec3 fragPosWorldSpaceInTCS[];
in vec2 texCoordInTCS[];
in vec3 normalInTCS[];

out vec3 fragPosWorldSpaceInTES[];
out vec2 texCoordInTES[];
out vec3 normalInTES[];

float getTessellationLevel(vec2 texCoord) {
    vec2 texCoordFlippedY = texCoord;
    texCoordFlippedY.y = -texCoordFlippedY.y;
    float sampledHeight = texture(heightmap, texCoordFlippedY).r;
    return sampledHeight;
}

void main()
{
    texCoordInTES[gl_InvocationID] = texCoordInTCS[gl_InvocationID];
    normalInTES[gl_InvocationID] = normalInTCS[gl_InvocationID];
    fragPosWorldSpaceInTES[gl_InvocationID] = fragPosWorldSpaceInTCS[gl_InvocationID];

    float h1 = getTessellationLevel(texCoordInTES[0]);
    float h2 = getTessellationLevel(texCoordInTES[1]);
    float h3 = getTessellationLevel(texCoordInTES[2]);

    float d1 = abs(h1 - h2);
    float d2 = abs(h2 - h3);
    float d3 = abs(h3 - h1);

    int TL = 10;

    gl_TessLevelOuter[0] = TL;
    gl_TessLevelOuter[1] = TL;
    gl_TessLevelOuter[2] = TL;
    gl_TessLevelInner[0] = TL;
}