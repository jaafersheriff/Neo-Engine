
in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform sampler2D diffuseMap;
uniform vec3 diffuseColor;
uniform vec3 ambientColor;

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gDiffuse;

void main() {
    gNormal = vec4(normalize(fragNor), 1.0);
    gDiffuse = vec4(diffuseColor + texture(diffuseMap, fragTex).rgb, 1.f);
}  