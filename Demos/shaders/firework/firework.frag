
in float fIntensity;
in float fParent;

out vec4 color;

void main() {
	if (fParent > 0.9) {
		color = vec4(vec3(7.5 * fIntensity, 8.0 * fIntensity, 0), 1.0);
	}
	else {
		color = vec4(vec3(fIntensity) * 0.4, 1.0);
	}

}