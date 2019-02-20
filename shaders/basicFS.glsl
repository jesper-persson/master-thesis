#version 400

uniform sampler2D texture1;

in vec2 textCoordOutVs;
in vec3 normalOutVs;
in vec3 fragPosWorldSpaceOutVs;

out vec4 colorOutFs;

void main() {
    vec3 lightPosition = vec3(20, 20, 20);
    vec3 directionToLight = normalize(lightPosition - fragPosWorldSpaceOutVs);
    float intensity = dot(normalize(normalOutVs), directionToLight);
    intensity = max(0, intensity);

    vec4 color = texture(texture1, textCoordOutVs);
    colorOutFs = color * 1;
}