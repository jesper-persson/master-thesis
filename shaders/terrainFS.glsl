#version 400

uniform sampler2D texture1;

vec3 lightPosition = vec3(-20, 40, 20);
vec3 lightDir = normalize(vec3(-1, -1, -1));

uniform sampler2D normalMap;
uniform sampler2D normalMapMacro;
uniform int normalMapRepeat;

uniform mat4 projection;

in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;
in vec3 cameraPosWorldSpaceInFS;
in mat3 TBNInFs;

out vec4 colorOutFs;

void main() {
    vec3 normal = normalize(normalInFS);
    vec3 normalBase = texture(normalMapMacro, texCoordInFS * vec2(1, -1)).rgb;
    vec3 normalDetail = texture(normalMap, texCoordInFS * normalMapRepeat).rgb;
    normalDetail = normalize(normalDetail * 2.0 - 1.0);
    normalDetail = normalize(TBNInFs * normalDetail);
    normal = normalize(normalBase);
    // normal = normalize(normalDetail + normalBase);

    // lightPosition = cameraPosWorldSpaceInFS;
    // vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    vec3 directionToLight = -1 * lightDir;
    vec3 directionToCamera = normalize(cameraPosWorldSpaceInFS - fragPosWorldSpaceInFS);

    float ambient = 0.73;
    float diffuseAmount = 0.20;
    float specularAmount = 0.001;
    vec4 color = vec4(0.99, 0.99, 1, 1);

    // If sand
    // ambient = 0.4;
    // diffuseAmount = 0.6;
    // specularAmount = 0.01;
    // color = vec4(0.68, 0.51, 0.28, 1);

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
}