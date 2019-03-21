#version 400

layout (vertices = 3) out;

uniform sampler2D heightmap;
uniform sampler2D normalMapMacro;

uniform mat4 worldToCamera;
uniform mat4 projection;

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

    // vec3 normal = normalize(texture(normalMapMacro, texCoordFlippedY).xyz);
    // vec3 up = vec3(0, 1, 0); // is y up though?
    // float res = 1 - abs(dot(normal, up)); // 0 when parallel, 1 when orthgonal
    // float level = (res * 10 + 1);

    return sampledHeight;
}

float getTessellationFactor(vec3 p1, vec3 p2) {
    float distanceSphere = length(p1 - p2);
    vec3 origin = (p1 + p2) / 2;

    vec4 clipPosition = projection * worldToCamera * vec4(origin, 1);
    float ll = abs(distanceSphere * projection[2][2] / clipPosition.w);

    float edgesPerScreenHeight = 20;

    return clamp(ll * edgesPerScreenHeight, 1, 12);
}

float tessellationFromCameraDistance(vec3 p1, vec3 p2) {
    mat4 worldToCameraInv = inverse(worldToCamera);
    vec3 cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);
    vec3 midPoint = (p1 + p2) / 2;
    float dist = length(midPoint - cameraPosWorldSpaceInFS);
    float tessellationFactor = -0.25*dist + 20;
    return clamp(tessellationFactor, 1, 64);
}

void main()
{
    texCoordInTES[gl_InvocationID] = texCoordInTCS[gl_InvocationID];
    normalInTES[gl_InvocationID] = normalInTCS[gl_InvocationID];
    fragPosWorldSpaceInTES[gl_InvocationID] = fragPosWorldSpaceInTCS[gl_InvocationID];

    float h1 = getTessellationLevel(texCoordInTES[0]);
    float h2 = getTessellationLevel(texCoordInTES[1]);
    float h3 = getTessellationLevel(texCoordInTES[2]);

    vec3 v1 = fragPosWorldSpaceInTCS[0] + vec3(0, h1, 0);
    vec3 v2 = fragPosWorldSpaceInTCS[1] + vec3(0, h2, 0);
    vec3 v3 = fragPosWorldSpaceInTCS[2] + vec3(0, h3, 0);
    
    float l1 = tessellationFromCameraDistance(v1, v2);
    float l2 = tessellationFromCameraDistance(v2, v3);
    float l3 = tessellationFromCameraDistance(v3, v1);

    float i = (l1 + l2 + l3 / 3.0);

    gl_TessLevelOuter[0] = l2;
    gl_TessLevelOuter[1] = l3;
    gl_TessLevelOuter[2] = l1;
    gl_TessLevelInner[0] = i;

    // gl_TessLevelOuter[0] = 10;
    // gl_TessLevelOuter[1] = 10;
    // gl_TessLevelOuter[2] = 10;
    // gl_TessLevelInner[0] = 10;
}