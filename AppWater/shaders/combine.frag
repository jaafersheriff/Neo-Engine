
#include "postprocess.glsl"

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gDepth;

uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 deferredColor = texture(gDiffuse, fragTex);
    vec4 deferredNormal = texture(gNormal, fragTex);
    float deferredDepth = texture(gDepth, fragTex).r;

    vec4 waterOutput = texture(inputFBO, fragTex);
    float waterDepth = texture(inputDepth, fragTex).r;

    color.rgb = (length(deferredNormal) <= 1.0 ? 1.f : 0.f) * deferredColor.rgb; 
    color.rgb += (length(deferredNormal) == 0.0 ? 0.f : 1.f) * (diffuseAmount * deferredColor.rgb + lightOutput.rgb);
    if (waterDepth < deferredDepth) {
       color.rgb = waterOutput.rgb * waterOutput.a + color.rgb * (1.0 - waterOutput.a);
    }
    color.a = 1.f;
}