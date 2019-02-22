#version 400

layout(triangles, equal_spacing, ccw) in;

uniform sampler2D heightmap;
uniform sampler2D normalmap;

uniform mat4 worldToCamera;
uniform mat4 projection;

in vec3 fragPosWorldSpaceInTES[];
in vec2 texCoordInTES[];
in vec3 normalInTES[];

out vec3 fragPosWorldSpaceInFS;
out vec2 texCoordInFS;
out vec3 normalInFS;
out vec3 cameraPosWorldSpaceInFS;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
   	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
   	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
   	texCoordInFS = interpolate2D(texCoordInTES[0], texCoordInTES[1], texCoordInTES[2]);
	vec2 texCoordFlippedY = texCoordInFS;
	texCoordFlippedY.y = -texCoordFlippedY.y;
	
   	normalInFS = interpolate3D(normalInTES[0], normalInTES[1], normalInTES[2]);
   	normalInFS = normalize(normalInFS);
	normalInFS = texture(normalmap, texCoordFlippedY).rgb;

   	fragPosWorldSpaceInFS = interpolate3D(fragPosWorldSpaceInTES[0], fragPosWorldSpaceInTES[1], fragPosWorldSpaceInTES[2]);
	float sampledHeight = texture(heightmap, texCoordFlippedY).r;
	fragPosWorldSpaceInFS.y = sampledHeight * 10.0f;

   	gl_Position = projection * worldToCamera * vec4(fragPosWorldSpaceInFS, 1.0);

    mat4 worldToCameraInv = inverse(worldToCamera);
    cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);
}