
in float fIntensity;
in float fParent;

uniform vec3 parentColor;

out vec4 color;

void main() {
	if (fParent > 0.9) {
		color.rgb = parentColor * fIntensity;
		color.a = 1.0;
	}
	else {
		color = vec4(vec3(fIntensity) * 0.4, 1.0);
	}

}