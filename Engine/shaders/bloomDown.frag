in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;
uniform vec2 texelSize;

out vec4 color;

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
void main() {
	float x = texelSize.x;
	float y = texelSize.y;

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(inputTexture, vec2(fragTex.x - 2 * x, fragTex.y + 2 * y)).rgb;
	vec3 b = texture(inputTexture, vec2(fragTex.x, fragTex.y + 2 * y)).rgb;
	vec3 c = texture(inputTexture, vec2(fragTex.x + 2 * x, fragTex.y + 2 * y)).rgb;

	vec3 d = texture(inputTexture, vec2(fragTex.x - 2 * x, fragTex.y)).rgb;
	vec3 e = texture(inputTexture, vec2(fragTex.x, fragTex.y)).rgb;
	vec3 f = texture(inputTexture, vec2(fragTex.x + 2 * x, fragTex.y)).rgb;

	vec3 g = texture(inputTexture, vec2(fragTex.x - 2 * x, fragTex.y - 2 * y)).rgb;
	vec3 h = texture(inputTexture, vec2(fragTex.x, fragTex.y - 2 * y)).rgb;
	vec3 i = texture(inputTexture, vec2(fragTex.x + 2 * x, fragTex.y - 2 * y)).rgb;

	vec3 j = texture(inputTexture, vec2(fragTex.x - x, fragTex.y + y)).rgb;
	vec3 k = texture(inputTexture, vec2(fragTex.x + x, fragTex.y + y)).rgb;
	vec3 l = texture(inputTexture, vec2(fragTex.x - x, fragTex.y - y)).rgb;
	vec3 m = texture(inputTexture, vec2(fragTex.x + x, fragTex.y - y)).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1
	vec3 downsample = e * 0.125;
	downsample += (a + c + g + i) * 0.03125;
	downsample += (b + d + f + h) * 0.0625;
	downsample += (j + k + l + m) * 0.125;

	color = vec4(downsample, 1.0);
}