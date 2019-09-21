#include "postprocess.glsl"

uniform sampler2D godray;

out vec4 color;

void main() {

    color = texture(inputFBO, fragTex) + vec4(texture(godray, fragTex).r);
    
}