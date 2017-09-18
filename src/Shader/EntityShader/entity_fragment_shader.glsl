#version 400 core

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float shine;

uniform vec3 lightPos;
uniform vec3 lightCol;

in vec3 fragNormal;
in vec3 worldPos;

out vec4 color;

void main() {
   vec3 lightDir = normalize(lightPos - worldPos);
   vec3 viewDir = normalize(-worldPos);

   vec3 ambientColor = matAmbient * lightCol;
   vec3 diffuseColor = matDiffuse * max(dot(lightDir, fragNormal), 0) * lightCol;

   // Blinn-Phong
   vec3 H = (lightDir + viewDir) / 2;
   vec3 specularColor = matSpecular * pow(max(dot(H, fragNormal), 0), shine) * lightCol;

   color = vec4(specularColor + diffuseColor + ambientColor, 1.0);
}