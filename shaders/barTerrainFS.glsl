#version 400

uniform sampler2D texture1;

in float barHeightInFS;

out vec4 colorOutFs;

void main() {
    colorOutFs = vec4(1,1,1,1) * (barHeightInFS - 2) / 8.0;
}