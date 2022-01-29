
in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;

uniform bool showAO;
uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 aoOutput = texture(inputFBO, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    color.rgb = diffuseAmount * diffuse.rgb + (lightOutput.rgb * (showAO ? aoOutput.r : 1.f));
    color.a = 1.f;
}