in vec2 fragTex;

layout(binding = 0) uniform sampler2D bloomResults;
layout(binding = 1) uniform sampler2D hdrColor;

out vec4 color;

void main() {
	color = mix(texture(bloomResults, fragTex), texture(hdrColor, fragTex), 0.04);
}