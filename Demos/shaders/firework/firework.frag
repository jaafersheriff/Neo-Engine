
in float fIntensity;
in float fParent;

out vec4 color;

void main() {
	if (fParent > 0.9) {
		color = vec4(vec3(0.8 * fIntensity, 0.6 * fIntensity, 0), 1);
	}
	else {
		color = vec4(vec3(fIntensity), 1);
	}

}