#version 330 core

in vec3 worldPos

uniform sampler2D glow;
uniform sampler2D color;

uniform vec3 lightPos;

out vec4 color;

void main() {
    vec3 V = normalize(worldPos);   // TODO : vertex pos or world pos?
    vec3 L = normalize(lightPos);

    float vDotL = dot(V, L);

    vec4 Kc = texture2D(color, vec2((L.y + 1.0) / 2.0, V.y));
    vec4 Kg = texture2D(glow,  vec2((L.y + 1.0) / 2.0, vl));

    color = vec4(Kc.rgb + Kg.rgb * Kg.a / 2.0, Kc.a);
}