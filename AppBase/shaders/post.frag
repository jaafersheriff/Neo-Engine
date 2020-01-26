#include "postprocess.glsl"

uniform sampler2D dofDown;
uniform sampler2D dofNearBlur;
uniform sampler2D dofInterpolate;

out vec4 color;

void main() { 
    color = texture(dofInterpolate, fragTex); 
}
