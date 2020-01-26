#include "postprocess.glsl"

uniform sampler2D dofDown;
uniform sampler2D dofBlur;

out vec4 color;

void main() { 
    color = texture(dofBlur, fragTex); 
}
