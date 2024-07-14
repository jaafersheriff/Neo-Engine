
in float fIntensity;

out vec4 color;

void main() {
	color = vec4(vec3(fIntensity), 1);
}