#include "postprocess.glsl"

uniform sampler2D godray;
uniform float exposure;

out vec4 color;

void main() {

    color = texture(inputFBO, fragTex) + vec4(texture(godray, fragTex).r) * exposure;
    
}