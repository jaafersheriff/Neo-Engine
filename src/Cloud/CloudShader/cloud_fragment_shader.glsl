#version 330 core

in vec3 worldPos;
in vec2 textureCoords;

uniform sampler2D textureImage;
uniform vec3 lightPos;
uniform vec3 lightCol;

out vec4 color;

#define PI 3.1415926

void main() {
    vec3 lightDir = lightPos - worldPos;
    vec3 L = normalize(lightDir);
    vec3 N = (vec3(
                -cos(textureCoords.x * PI),
                -cos(textureCoords.y * PI),
                sin(textureCoords.x * PI) * sin(textureCoords.y * PI)));
    vec3 diffuseContrib = max(dot(N, L), 0.0) * lightCol;
    vec4 texel = texture(textureImage, textureCoords);

    color = vec4(diffuseContrib, 1.0);
}
