#include "postprocess.glsl"

uniform sampler2D dofdown;

out vec4 color;

void main() { 
    color = texture(dofdown, fragTex); 
}
