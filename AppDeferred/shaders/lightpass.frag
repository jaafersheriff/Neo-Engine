#version 330 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
uniform sampler2D gDepth;

uniform vec3 lightPos;
uniform vec3 lightCol;

out vec4 color;

void main() {
    color = vec4(lightCol, 1.f);
}