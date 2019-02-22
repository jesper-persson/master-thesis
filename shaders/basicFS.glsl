#version 400

uniform sampler2D texture1;

in vec2 texCoordInFS;
in vec3 normalInFS;
in vec3 fragPosWorldSpaceInFS;

out vec4 colorOutFs;

void main() {
    vec3 lightPosition = vec3(20, 20, 20);
    vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceInFS);
    float intensity = dot(normalize(normalInFS), directionToLight);
    intensity = max(0, intensity);

    vec4 color = texture(texture1, texCoordInFS);
    colorOutFs = color * intensity;
}