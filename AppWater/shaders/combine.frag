
#include "postprocess.glsl"

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;
uniform sampler2D gDepth;

uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    vec4 waterOutput = texture(inputFBO, fragTex);
    float waterDepth = texture(inputDepth, fragTex).r;
    vec3 diff = diffuseAmount * diffuse.rgb + lightOutput.rgb;
    if (waterDepth < 1.0 && texture(gDepth, fragTex).r > waterDepth) {
       color.rgb = waterOutput.rgb * waterOutput.a;
    }
    else {
        color.rgb = diff;
    }
    color.a = 1.f;
}