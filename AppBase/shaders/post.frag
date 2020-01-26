#include "postprocess.glsl"

uniform sampler2D DOFA;

out vec4 color;

void main() { 
    color = texture(DOFA, fragTex); 
}
