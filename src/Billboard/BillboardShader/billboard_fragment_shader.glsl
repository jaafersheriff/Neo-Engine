#version 330 core

in vec3 textureCoords;

uniform sampler2D textureImage;

out vec4 color;

void main() {
    color = texture(textureImage, textureCoords.xy);
}