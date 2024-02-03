
in vec2 fragTex;

layout(binding = 0) uniform sampler2D lightOutput;
layout(binding = 1) uniform sampler2D aoOutput;

out vec4 outcolor;

void main() {
	vec3 lightResolve = texture(lightOutput, fragTex).rgb;
	lightResolve *= texture(aoOutput, fragTex).r;

	outcolor = vec4(lightResolve, 1.0);
}
