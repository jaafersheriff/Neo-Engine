#include "postprocess.glsl"

uniform sampler2D dofDown;
uniform sampler2D dofNearBlur;

out vec4 color;

void main() { 
    color = texture(dofNearBlur, fragTex); 
}
