#version 330 core

in vec3 worldPos;
in vec3 viewDir;
in vec2 textureCoords;

uniform sampler2D textureImage;

out vec4 color;

#define PI 3.1415926

void main() {
    vec3 lightPos = vec3(-10, 100, 10);
    vec3 lightDir = lightPos - worldPos;
    vec3 L = normalize(lightDir);
    vec3 sphereNormal = vec3( 
        -cos(textureCoords.x * PI),
        -cos(textureCoords.y * PI),
        sin(textureCoords.x * PI) * sin(textureCoords.y * PI));
    vec3 N = normalize(sphereNormal);
    vec3 diffuseContrib = max(dot(N, L), 0.0) * vec3(1, 1, 1);
    vec4 texel = texture(textureImage, textureCoords);

    color = vec4(N, 1.0);
}
