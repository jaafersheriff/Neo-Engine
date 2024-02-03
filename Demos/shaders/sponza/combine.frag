
in vec2 fragTex;

layout(binding = 0) uniform sampler2D lightOutput;

#ifdef DRAW_AO
layout(binding = 1) uniform sampler2D aoOutput;
#endif

out vec4 outcolor;

void main() {
	vec3 lightResolve = texture(lightOutput, fragTex).rgb;
#ifdef DRAW_AO
	lightResolve *= texture(aoOutput, fragTex).r;
#endif

	outcolor = vec4(lightResolve, 1.0);
}
