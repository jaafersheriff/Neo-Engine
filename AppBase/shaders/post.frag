#include "postprocess.glsl"

uniform sampler2D dofblur;

// TODO - allow for variable num taps using texture hax
#define NUM_TAPS 8
uniform vec2 poisson[NUM_TAPS]; // poisson-distributed positions on the unit circle

uniform vec2 scenePixelSize; // pixel size ( 1 / image resolution ) of full resolution image
uniform vec2 dofPixelSize; // pixel size of dofblur

uniform vec2 maxCoc; // max circle of confusion radius and diameter (in pixels)

uniform float radiusScale; // scale factor for max coc size on dofblur

out vec4 color;

void main() { 
    vec4 sceneColor = texture(inputFBO, fragTex);
    vec4 dofColor = texture(dofblur, fragTex);

    color = dofColor;
}
