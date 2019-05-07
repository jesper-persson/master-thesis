#version 400

uniform sampler2D texture1;

/**
 * Set to 0 to disable, set to 1 to enable
*/
uniform int useNormalMapping;
uniform sampler2D normalMap;
uniform int normalMapRepeat;

in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;
in vec3 cameraPosWorldSpaceInFS;
in mat3 TBNInFs;

int WINDOW_HEIGHT = 1200;
int WINDOW_WIDTH = 1800;

out vec4 colorOutFs;

void main() {
    vec3 normal = normalize(normalInFS);
    if (useNormalMapping > 0) {
        normal = texture(normalMap, texCoordInFS * normalMapRepeat).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(TBNInFs * normal);
    }

    vec3 lightPosition = vec3(-20, 40, 20);
    // lightPosition = cameraPosWorldSpaceInFS;
    vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    vec3 directionToCamera = normalize(cameraPosWorldSpaceInFS - fragPosWorldSpaceInFS);

    float ambient = 0.4;

    // Diffuse
    float intensity = dot(normal, directionToLight);
    intensity = max(0, intensity) *1;

    // Specular lighting
    vec3 reflectionVector = reflect(-directionToLight, normal);
    float beta = dot(reflectionVector, directionToCamera);
    float cosAngle = max(0.0, dot(directionToCamera, reflectionVector));
    float shininess = 1000;
    float specularCoefficient = pow(cosAngle, shininess) * 1;

    vec4 color = texture(texture1, texCoordInFS);
    colorOutFs = color * (intensity + specularCoefficient + ambient);
}