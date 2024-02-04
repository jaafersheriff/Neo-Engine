
in vec2 fragTex;

uniform vec3 spriteColor;

out vec4 color;

void main() {
	// Quick fall-off computation
	float r = length(fragTex*2.0-1.0)*3.0;
	float i = exp(-r*r);
	if (i < 0.01) discard;

	color = vec4(spriteColor, i);
}