
#include "postprocess.glsl"

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;
uniform sampler2D decals;

uniform bool showAO;
uniform float diffuseAmount;

out vec4 outcolor;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 aoOutput = texture(inputFBO, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    vec4 decal = texture(decals, fragTex);

    outcolor.rgb = 
	 	diffuseAmount * diffuse.rgb
		+ diffuseAmount * decal.rgb
		+ lightOutput.rgb * decal.rgb
		+ (lightOutput.rgb * (showAO ? aoOutput.r : 1.f));
    outcolor.a = 1.f;
}
