#version 400

uniform sampler2D texture1;
uniform sampler2D shadowMap;
vec3 lightPosition = vec3(20, 20, 20);
vec3 lightDir = normalize(vec3(-1, -1, -1));

uniform sampler2D normalMap;
uniform sampler2D normalMapMacro;
uniform int normalMapRepeat;

uniform sampler2D occlusionMap;

uniform mat4 projection;

in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;
in vec3 fragPosViewSpaceInFS;
in vec3 cameraPosWorldSpaceInFS;
in mat3 TBNInFs;

in vec3 shadowCoordInFS;

out vec4 colorOutFs;

// Needed for SSAO texture lookup
float WINDOW_HEIGHT = 1200;
float WINDOW_WIDTH = 1800;

float depthValueForViewSpaceCoord(vec3 viewSpaceCoord) {
    vec4 coord = vec4(viewSpaceCoord, 1);
    coord = projection * coord;
    coord.xyz /= coord.w;
    coord.xy = coord.xy * 0.5 + 0.5;
    return texture(occlusionMap, coord.xy).r;
}

vec3 getNormalWeights(vec3 normal){
	vec3 weights = abs(normal);
	weights = normalize(max(weights, 0.00001));
	weights /= vec3(weights.x + weights.y + weights.z);
	return weights;
}

void main() {
    vec3 normal = normalize(normalInFS);
    vec3 normalBase = texture(normalMapMacro, texCoordInFS * vec2(1, -1)).rgb;
    vec3 normalDetail = texture(normalMap, texCoordInFS * normalMapRepeat * 5).rgb * 1;
    normalDetail = normalize(normalDetail * 2.0 - 1.0);
    normalDetail = normalize(TBNInFs * normalDetail);
    normal = normalize(normalBase);

    // lightPosition = cameraPosWorldSpaceInFS;
    // vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    vec3 directionToLight = -1*lightDir;
    vec3 directionToCamera = normalize(cameraPosWorldSpaceInFS - fragPosWorldSpaceInFS);

    float ambient = 0.8;
    float diffuseAmount = 0.25;
    float specularAmount = 0.01;
    vec4 color = vec4(0.98, 0.98, 1, 1);

    // If sand
    // ambient = 0.4;
    // diffuseAmount = 0.6;
    // specularAmount = 0.01;
    // color = vec4(0.68, 0.51, 0.28, 1);

    // SSAO
    // float occlusionFactor = depthValueForViewSpaceCoord(fragPosViewSpaceInFS);
    // occlusionFactor = 1 - texture(occlusionMap, gl_FragCoord.xy / vec2(WINDOW_WIDTH, WINDOW_HEIGHT)).r;
    float occlusionFactor = 0;
    ambient = ambient -  ambient * (occlusionFactor / 4.0);

    // Diffuse
    float intensity = dot(normal, directionToLight);
    intensity = max(0, intensity) * diffuseAmount;

    // Specular lighting
    vec3 reflectionVector = reflect(-directionToLight, normal);
    float beta = dot(reflectionVector, directionToCamera);
    float cosAngle = max(0.0, dot(directionToCamera, reflectionVector));
    float shininess = 100;
    float specularCoefficient = pow(cosAngle, shininess) * specularAmount;

    // vec4 color = texture(texture1, texCoordInFS);
    colorOutFs = color * (intensity + specularCoefficient + ambient);

    // Shadow
    float visibility = 1;
    float bias = 0.005;
    bias = max(0.05 * (1.0 - dot(normal, directionToLight)), 0.005);  
    if (texture(shadowMap, shadowCoordInFS.xy).r < shadowCoordInFS.z - bias) {
        visibility = 1;
    }

    colorOutFs *= visibility;

    // Tri planar
    // vec3 normalWeights = blendNormal(normalize(normalBase));
    // vec3 xColor = texture(normalMap, fragPosWorldSpaceInFS.yz * 1).rgb;
    // vec3 yColor = texture(normalMap, fragPosWorldSpaceInFS.xz * 1).rgb;
    // vec3 zColor = texture(normalMap, fragPosWorldSpaceInFS.xy * 1).rgb;
    // vec3 finalColor = (xColor * normalWeights.x + yColor * normalWeights.y + zColor * normalWeights.z);
    // colorOutFs = vec4(finalColor, 1) * (intensity + specularCoefficient + ambient);
}