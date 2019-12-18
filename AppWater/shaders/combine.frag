
#include "postprocess.glsl"

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;
uniform sampler2D gDepth;

uniform float diffuseAmount;

out vec4 color;

void main() {
    if(texture2D(gDepth, fragTex).r > texture2D(inputDepth, fragTex).r) {
        color = vec4(texture(inputFBO, fragTex).rgb, 1.0);
        return;
    }
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    color.rgb = diffuseAmount * diffuse.rgb + lightOutput.rgb;
    color.a = 1.f;
}