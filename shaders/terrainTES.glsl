#version 400

layout(triangles, fractional_odd_spacing, ccw) in;

uniform sampler2D heightmap;

uniform mat4 worldToCamera;
uniform mat4 projection;

in vec3 fragPosWorldSpaceInTES[];
in vec2 texCoordInTES[];
in vec3 normalInTES[];

out vec3 fragPosWorldSpaceInFS;
out vec2 texCoordInFS;
out vec3 normalInFS;
out vec3 cameraPosWorldSpaceInFS;
out mat3 TBNInFs;

uniform int heightColumnScale;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
   	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2) {
   	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main() {
   	texCoordInFS = interpolate2D(texCoordInTES[0], texCoordInTES[1], texCoordInTES[2]);
	vec2 texCoordFlippedY = texCoordInFS;
	texCoordFlippedY.y = -texCoordFlippedY.y;
	
   	normalInFS = interpolate3D(normalInTES[0], normalInTES[1], normalInTES[2]);

   	fragPosWorldSpaceInFS = interpolate3D(fragPosWorldSpaceInTES[0], fragPosWorldSpaceInTES[1], fragPosWorldSpaceInTES[2]);
	// uint sampledHeight = texture(heightmap, texCoordFlippedY).r;
	// fragPosWorldSpaceInFS.y = sampledHeight/float(heightColumnScale);
	float sampledHeight = texture(heightmap, texCoordFlippedY).r;
	fragPosWorldSpaceInFS.y = sampledHeight;

   	gl_Position = projection * worldToCamera * vec4(fragPosWorldSpaceInFS, 1.0);

    mat4 worldToCameraInv = inverse(worldToCamera);
    cameraPosWorldSpaceInFS = vec3(worldToCameraInv[3][0], worldToCameraInv[3][1], worldToCameraInv[3][2]);

    TBNInFs = mat3(vec3(1, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0));
}