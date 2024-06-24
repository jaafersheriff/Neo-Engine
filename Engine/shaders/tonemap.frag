in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;

out vec4 color;

void main() {

	color = linearToSrgb(texture(inputTexture, fragTex));
}