#version 330

in vec2 textureCoords;

uniform sampler2D textureImage;

out vec4 color;

void main() {
    color = texture(textureImage, textureCoords);
}