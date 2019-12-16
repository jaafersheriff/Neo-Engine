
#include "postprocess.glsl"

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;

uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 forwardOutput = texture(inputFBO, fragTex);
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    color.rgb = forwardOutput.rgb * forwardOutput.a+ diffuseAmount * diffuse.rgb * (1.0 - forwardOutput.a) + lightOutput.rgb;
    color.a = 1.f;
}