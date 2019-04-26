#version 400

uniform isampler2D texture1;

in vec2 texCoordInFS;

out vec4 colorOutFS;

void main() {
    // vec4 color = texture(texture1, texCoordInFS) / 1;
    // colorOutFS = vec4(1,1,1,1) * color.a;// / 64;
    // colorOutFS = color * -10;

    // colorOutFS = vec4(1,1,1,1) * float(texture(texture1,texCoordInFS).z) / 5000;

    if (texture(texture1,texCoordInFS).z == -2) {
        colorOutFS = vec4(1,0,0,1);
    } else if (texture(texture1,texCoordInFS).z == -3) {

        colorOutFS = vec4(0,1,0,1);
    } else {
        colorOutFS = vec4(0,0,1,1);
        
    }
    
}