
in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputAO;

uniform int blurAmount;

out vec4 color;

void main() {
	if (blurAmount == 0) {
		color = texture(inputAO, fragTex);
		return;
	}

	// TODO - pass in the resolution as a unsiform
	vec2 texelSize = 1.f / vec2(textureSize(inputAO, 0));
	float result = 0.f;
	for (int x = -blurAmount; x < blurAmount; x++) {
		for (int y = -blurAmount; y < blurAmount; y++) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(inputAO, fragTex + offset).r;
		}
	}
	color = vec4(result / (4.0 * blurAmount * blurAmount));
}