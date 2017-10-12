#version 330 core

uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float shine;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform sampler2D textureImage;
uniform bool usesTexture;

in vec4 worldPos;
in vec3 fragNormal;
in vec4 viewDir;
in vec2 pass_textureCoords;

out vec4 color;

void main() {
   vec3 lightDir = lightPos - worldPos.xyz;
   vec3 unitViewDir = normalize(viewDir.xyz);
   vec3 unitLightDir = normalize(lightDir);
   vec3 unitNormal = normalize(fragNormal);

   float lightDistance = length(lightDir);
   float attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;

   vec3 diffuseContrib = max(dot(unitLightDir, unitNormal), 0.0) * lightCol / attFactor;

   // Blinn-Phong
   vec3 H = (unitLightDir + unitViewDir) / 2;
   vec3 specularContrib = pow(max(dot(H, unitNormal), 0), shine) * lightCol / attFactor;

   vec3 diffuseColor = matDiffuse;
   if (usesTexture) {
      diffuseColor = vec3(texture(textureImage, pass_textureCoords));
   }

   color = vec4(diffuseColor*diffuseContrib+matSpecular*specularContrib, 1);
}