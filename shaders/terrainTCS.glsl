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

float getTerrainHeight(vec2 texCoord) {
    vec2 texCoordFlippedY = texCoord;
    texCoordFlippedY.y = -texCoordFlippedY.y;
    // float sampledHeight = texture(heightmap, texCoordFlippedY).r / float(heightColumnScale);
    float sampledHeight = texture(heightmap, texCoordFlippedY).r;
    return sampledHeight;
}

const int maxTessellation = 16;
const int minTessellation = 2;

const float distanceMaxTess = 15;
const float distanceMinTess = 100;

// p1 and p2 are two endpoints of a edge
float tessellationFromCameraDistance(vec3 p1, vec3 p2) {
    mat4 worldToCameraInv = inverse(worldToCamera);
    vec3 cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);
    vec3 midPoint = (p1 + p2) / 2;
    float dist = length(midPoint - cameraPosWorldSpaceInFS);

    float slope = (minTessellation - maxTessellation)/(distanceMinTess - distanceMaxTess);
    float tessellationFactor = slope*(dist - distanceMaxTess) + maxTessellation;

    return clamp(tessellationFactor, minTessellation, maxTessellation);
}

void main() {
    texCoordInTES[gl_InvocationID] = texCoordInTCS[gl_InvocationID];
    normalInTES[gl_InvocationID] = normalInTCS[gl_InvocationID];
    fragPosWorldSpaceInTES[gl_InvocationID] = fragPosWorldSpaceInTCS[gl_InvocationID];

    float h1 = getTerrainHeight(texCoordInTES[0]);
    float h2 = getTerrainHeight(texCoordInTES[1]);
    float h3 = getTerrainHeight(texCoordInTES[2]);

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

    // gl_TessLevelOuter[0] = 16;
    // gl_TessLevelOuter[1] = 16;
    // gl_TessLevelOuter[2] = 16;
    // gl_TessLevelInner[0] = 16;
}