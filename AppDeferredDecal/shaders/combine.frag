#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;
uniform sampler2D gDiffuse;
uniform sampler2D decals;

uniform float diffuseAmount;

out vec4 color;

void main() {
    vec4 lightOutput = texture(lightOutput, fragTex);
    vec4 diffuse = texture(gDiffuse, fragTex);
    vec4 decal = texture(decals, fragTex);
    vec4 scene = texture(inputFBO, fragTex);
    color.rgb = diffuseAmount * diffuse.rgb + lightOutput.rgb + decal.rgb + scene.rgb;
    color.a = 1.f;
}