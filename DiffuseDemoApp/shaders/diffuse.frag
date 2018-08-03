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
    float diffuseContrib = max(lambert, 0.0f);
    float specularContrib = pow(max(dot(H, N), 0.0), shine);

    color = texColor * ambient + // ambient
            texColor * diffuseContrib + // diffuse
            vec4(specularColor, 0) * specularContrib;  // specular
    color.a = 1;
}