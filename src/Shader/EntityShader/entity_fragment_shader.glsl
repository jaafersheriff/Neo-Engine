#version 330 core

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float shine;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

in vec3 fragNormal;
in vec4 worldPos;

out vec4 color;

void main() {
   vec3 lightDir = lightPos - worldPos.xyz;
   vec3 unitViewDir = normalize(-worldPos.xyz);
   vec3 unitLightDir = normalize(lightDir);

   float lightDistance = length(lightDir);
   float attFactor = lightAtt.x + lightAtt.y * lightDistance + lightAtt.z * lightDistance * lightDistance;

   vec3 ambientContrib = matAmbient * lightCol / attFactor;
   vec3 diffuseContrib = matDiffuse * max(dot(unitLightDir, fragNormal), 0) * lightCol / attFactor;

   // Blinn-Phong
   vec3 H = (unitLightDir + unitViewDir) / 2;
   vec3 specularContrib = matSpecular * pow(max(dot(H, fragNormal), 0), shine) * lightCol / attFactor;

   color = vec4(ambientContrib*matAmbient + diffuseContrib*matDiffuse + specularContrib*matSpecular, 1.0);
}