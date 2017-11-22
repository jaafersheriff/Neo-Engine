#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexture;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

uniform float uTime;

uniform float matAmbient;
uniform float shine;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

out vec2 pass_textureCoords;
out vec3 diffuseContrib;
out vec3 specularContrib;

void main() {
    /* Per-vertex Blinn Phong */
    vec4 worldPos = M * vertexPos;
    vec3 lightDir = lightPos - worldPos.xyz;
    vec3 L = normalize(lightDir);
    vec3 V = normalize(vec3((inverse(V) * vec4(0.0, 0.0, 0.0, 1.0)) - worldPos));
    vec3 N = normalize(vec3(M * vec4(vertexNormal, 0.0)));
    vec3 H = (L + V) / 2;
    
    float lightDistance = length(lightDir);
    float attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;

    diffuseContrib = max(dot(L, N), matAmbient) * lightCol / attFactor;
    specularContrib = pow(max(dot(H, N), 0), shine) * lightCol / attFactor;
    pass_textureCoords = vertexTexture;
    
    gl_Position = P * V * worldPos;
}
