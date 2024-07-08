#include "color.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;
uniform vec2 texelSize;
uniform float threshold;

out vec4 color;

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
float KarisAverage(vec3 col) {
    // Formula is 1 / (1 + luma)
    float luma = luminance(linearToSrgb(vec4(col, 1.0)).rgb) * 0.25f;
    return 1.0f / (1.0f + luma);
}
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

	vec3 downsample = vec3(0.f);
#ifdef MIP_0
	vec3 groups[5];
	groups[0] = (a+b+d+e) * (0.125f/4.0f);
	groups[1] = (b+c+e+f) * (0.125f/4.0f);
	groups[2] = (d+e+g+h) * (0.125f/4.0f);
	groups[3] = (e+f+h+i) * (0.125f/4.0f);
	groups[4] = (j+k+l+m) * (0.5f/4.0f);
	groups[0] *= KarisAverage(groups[0]);
	groups[1] *= KarisAverage(groups[1]);
	groups[2] *= KarisAverage(groups[2]);
	groups[3] *= KarisAverage(groups[3]);
	groups[4] *= KarisAverage(groups[4]);
	downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
	if (luminance(downsample) <= threshold) {
		color = vec4(0, 0, 0, 1);
		return;
	}
#else
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
	downsample = e * 0.125;
	downsample += (a + c + g + i) * 0.03125;
	downsample += (b + d + f + h) * 0.0625;
	downsample += (j + k + l + m) * 0.125;
#endif

	color = vec4(downsample, 1.0);
}