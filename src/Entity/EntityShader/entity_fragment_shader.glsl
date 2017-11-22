#version 330 core

in vec2 pass_textureCoords;
in vec3 diffuseContrib;
in vec3 specularContrib;

uniform float matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;

uniform sampler2D textureImage;
uniform bool usesTexture;

out vec4 color;

void main() {
   vec3 diffuseColor = matDiffuse;
    if (usesTexture) {
        diffuseColor = vec3(texture(textureImage, pass_textureCoords));
    }

    color = vec4(diffuseColor*diffuseContrib+matSpecular*specularContrib, 1);
}