#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform bool useNormalMap;
uniform sampler2D normalMap;

uniform bool useDiffuseMap;
uniform sampler2D diffuseMap;
uniform vec3 diffuseMaterial;

uniform bool useSpecularMap;
uniform sampler2D specularMap;

// layout (location = 0) out vec3 gPosition;
// layout (location = 1) out vec3 gNormal;
// layout (location = 2) out vec4 gAlbedoSpec;

out vec4 color;

void main() {    
    color = vec4(1,0,1,1);
    // gPosition = vec3(1,0,0); //fragPos.xyz;
    // gNormal = useNormalMap ? texture(normalMap, fragTex).rgb : normalize(fragNor);
    // gAlbedoSpec.rgb = useDiffuseMap ? texture(diffuseMap, fragTex).rgb : diffuseMaterial;
    // gAlbedoSpec.a = useSpecularMap ? texture(specularMap, fragTex).r : 0.f;
}  