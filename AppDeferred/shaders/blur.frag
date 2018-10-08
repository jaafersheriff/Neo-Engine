#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform int blurAmount;

out vec4 color;

void main() {
    if (blurAmount == 0) {
        color = texture(inputFBO, fragTex);
        return;
    }

    vec2 texelSize = 1.f / vec2(textureSize(inputFBO, 0));
    float result = 0.f;
    vec2 hlim = vec2(float(-blurAmount) * 0.5f + 0.5f);
    for (int x = 0; x < blurAmount; x++) {
        for (int y = 0; y < blurAmount; y++) {
            vec2 offset = vec2(hlim + vec2(float(x), float(y))) * texelSize;
            result += texture(inputFBO, fragTex + offset).r;
        }
    }
    color.rgb = vec3(result / (blurAmount * blurAmount));
    color.a = 1.f;
}