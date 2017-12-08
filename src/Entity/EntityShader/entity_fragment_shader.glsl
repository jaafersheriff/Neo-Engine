#version 330 core

in vec4 worldPos;
in vec3 fragNormal;
in vec3 viewDir;
in vec2 textureCoords;

uniform float matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float shine;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform sampler2D textureImage;
uniform bool usesTexture;

out vec4 color;

void main() {
    vec3 lightDir = lightPos - worldPos.xyz;
    vec3 V = normalize(viewDir);
    vec3 L = normalize(lightDir);
    vec3 N = normalize(fragNormal);

    float lightDistance = length(lightDir);
    float attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;

    /* Diffuse */
    vec3 diffuseContrib = max(dot(L, N), matAmbient) * lightCol / attFactor;
    vec3 diffuseColor = matDiffuse;
    if (usesTexture) {
        diffuseColor = vec3(texture(textureImage, textureCoords));
    }

    /* Specualr using Blinn-Phong */
    vec3 H = (L + V) / 2;
    vec3 specularContrib = pow(max(dot(H, N), 0), shine) * lightCol / attFactor;

    color = vec4(diffuseColor*diffuseContrib+matSpecular*specularContrib, 1);
}