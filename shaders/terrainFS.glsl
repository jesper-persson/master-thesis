#version 400

uniform sampler2D texture1;

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

void main() {
    vec3 normal = normalize(normalInFS);
    vec3 normalBase = texture(normalMapMacro, texCoordInFS * vec2(1, -1)).rgb;
    vec3 normalDetail = texture(normalMap, texCoordInFS * normalMapRepeat * 0.1).rgb;
    normalDetail = normalize(normalDetail * 2.0 - 1.0);
    normalDetail = normalize(TBNInFs * normalDetail);
    normal = normalize(normalBase + normalDetail);

    vec3 lightPosition = vec3(20, 20, 20);
    // lightPosition = cameraPosWorldSpaceInFS;
    vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    vec3 directionToCamera = normalize(cameraPosWorldSpaceInFS - fragPosWorldSpaceInFS);

    float ambient = 0.7;

    // SSAO
    float occlusionFactor = depthValueForViewSpaceCoord(fragPosViewSpaceInFS);
    occlusionFactor = 1 - texture(occlusionMap, gl_FragCoord.xy / vec2(WINDOW_WIDTH, WINDOW_HEIGHT)).r;
    ambient = ambient -  ambient * (occlusionFactor / 4.0);

    // Diffuse
    float intensity = dot(normal, directionToLight);
    intensity = max(0, intensity) * 1 * 0.3 * 1;

    // Specular lighting
    vec3 reflectionVector = reflect(-directionToLight, normal);
    float beta = dot(reflectionVector, directionToCamera);
    float cosAngle = max(0.0, dot(directionToCamera, reflectionVector));
    float shininess = 100;
    float specularCoefficient = pow(cosAngle, shininess) * 0.2 * 1;

    vec4 color = texture(texture1, texCoordInFS);
    color = vec4(0.98, 0.98, 1, 1);
    colorOutFs = color * (intensity + specularCoefficient + ambient);
}