
in vec3 fragColor;

out vec4 color;

void main() {
	color = vec4(normalize(fragColor * 0.5 + 0.5), 1.0);
}  