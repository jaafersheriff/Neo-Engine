#version 330 core

in vec4 fragPos;
in vec3 fragNor;

uniform float ambient;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shine;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

out vec4 color;

void main() {
    vec3 N = normalize(fragNor);
    vec3 viewDir = camPos - fragPos.xyz;
    vec3 V = normalize(viewDir);
    vec3 lightDir = lightPos - fragPos.xyz;
    float lightDistance = length(lightDir);
    vec3 L = normalize(lightDir);

    float attFactor = 1;
    if (length(lightAtt) > 0) {
        attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
    }

    float lambert = dot(L, N);
    vec3 H = normalize(L + V);

    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f) / attFactor;
    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine) / attFactor;

    color.rgb = diffuseColor * ambient +
                diffuseColor * diffuseContrib +
                specularColor * specularContrib;
    color.a = 1;
}