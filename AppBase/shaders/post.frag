#include "postprocess.glsl"

uniform sampler2D inColor;
uniform float dofOpacity;

uniform bool showDebug;

out vec4 color;

void main() { 
    vec4 sceneColor = texture(inputFBO, fragTex);
    vec4 dofColor = texture(inColor, fragTex);
    color = sceneColor + dofColor * dofOpacity;
    if (showDebug) {
        color = dofColor;
    }
}
