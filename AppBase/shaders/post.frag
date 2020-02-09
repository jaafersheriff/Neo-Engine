#include "postprocess.glsl"

uniform sampler2D dofblur;

uniform int poissonSize;
uniform sampler1D poisson; // poisson-distributed positions on the unit circle

uniform vec2 scenePixelSize; // pixel size ( 1 / image resolution ) of full resolution image
uniform vec2 dofPixelSize; // pixel size of dofblur

uniform vec2 maxCoc; // max circle of confusion radius and diameter (in pixels)

uniform float radiusScale; // scale factor for max coc size on dofblur

out vec4 color;

void main() { 
    vec4 sceneColor = texture(inputFBO, fragTex);
    vec4 dofColor = texture(dofblur, fragTex);
    vec4 po = texture(poisson, fragTex.x);

    color = vec4(po.rg, 0.0, 1.0);
}   
