#version 330 core

in vec3 vertPos;

uniform sampler2D colorTexture;
uniform sampler2D glowTexture;

uniform vec3 lightPos;

out vec4 frag_color;

void main() {
    vec3 V = normalize(vertPos);
    vec3 L = normalize(lightPos);

    float LTex = (L.y + 1.0) / 2.0;
    float VTex = V.y;
    float vL = dot(V, L);

    // Look up the sky color and glow colors.
    vec4 Kc = texture(colorTexture, vec2(LTex, VTex));
    vec4 Kg = texture(glowTexture,  vec2(LTex, vL));

    // Combine the color and glow giving the pixel value.
    frag_color = vec4(Kc.rgb + Kg.rgb * Kg.a / 2.0, Kc.a);
}