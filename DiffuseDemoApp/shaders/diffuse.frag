#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform sampler2D diffuseMap;
uniform float ambient;
uniform vec3 specularColor;
uniform float shine;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

out vec4 color;

void main() {
    vec3 viewDir = camPos - fragPos.xyz;
    vec3 lightDir = lightPos - fragPos.xyz;
    vec3 V = normalize(viewDir);
    vec3 L = normalize(lightDir);
    vec3 N = normalize(fragNor);

    /* Base color */
    vec4 texColor = texture(diffuseMap, fragTex);
    if (texColor.a < 0.1) {
        discard; 
    }

    float lambert = dot(L, N);
    vec3 H = normalize(L + V);
    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f);
    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine);

    color.rgb = texColor.rgb * ambient + // ambient
                texColor.rgb * diffuseContrib + // diffuse
                specularColor * specularContrib;  // specular
    color.a = texColor.a;
}