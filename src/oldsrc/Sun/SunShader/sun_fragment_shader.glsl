#version 330

in vec3 worldPos;
in vec2 textureCoords;

uniform vec3 center;
uniform bool usesTexture;
uniform sampler2D textureImage;

uniform vec3 innerColor;
uniform vec3 outerColor;
uniform float innerRadius;
uniform float outerRadius;

out vec4 color;

void main() {
    if (usesTexture) {
        color = texture(textureImage, textureCoords);
        return;
    }

    float dist = distance(center, worldPos);
    if (dist < innerRadius) {
        color = vec4(innerColor, 1.0);
    }
    else {
        float scale = (dist - innerRadius) / (outerRadius - innerRadius);
        color = vec4(outerColor * scale + innerColor * (1 - scale), 1 - scale);
    }
}