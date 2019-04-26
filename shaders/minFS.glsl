#version 400

uniform sampler2D texture1;
uniform sampler2D texture2;

in vec2 texCoordInFS;

out vec4 result;

void main() {
    vec4 color1 = texture(texture1, texCoordInFS);
    vec4 color2 = texture(texture2, texCoordInFS);
    result = min(color1, color2);
    if (color2.r > 9.9) { // Probably no intersecting object
        result = color1;
    }
}