#version 400

uniform sampler2D texture1;

in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;
in vec3 cameraPosWorldSpaceInFS;

out vec4 colorOutFs;

void main() {
    vec3 lightPosition = vec3(20, 20, 20);
    // lightPosition = cameraPosWorldSpaceInFS;
    vec3 normal = normalize(normalInFS);
    vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    vec3 directionToCamera = normalize(cameraPosWorldSpaceInFS - fragPosWorldSpaceInFS);

    float ambient = 0.80;

    // Diffuse
    float intensity = dot(normal, directionToLight);
    intensity = max(0, intensity) * 0.2;

    // Specular lighting
    vec3 reflectionVector = reflect(-directionToLight, normal);
    float beta = dot(reflectionVector, directionToCamera);
    float cosAngle = max(0.0, dot(directionToCamera, reflectionVector));
    float shininess = 10;
    float specularCoefficient = pow(cosAngle, shininess) * 0.02;

    vec4 color = texture(texture1, texCoordInFS);
    colorOutFs = color * (intensity + specularCoefficient + ambient);
}