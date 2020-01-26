#include "postprocess.glsl"

uniform sampler2D inColor;

out vec4 color;

void main() { 
    color = texture(inColor, fragTex); 
}
