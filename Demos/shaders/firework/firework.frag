
in float fIntensity;
in float fParent;

uniform vec3 parentColor;
uniform vec3 childColor;
uniform float childColorBias;

out vec4 color;

void main() {
	if (fParent > 0.9) {
		color.rgb = parentColor * fIntensity;
		color.a = 1.0;
	}
	else {
		color.rgb = fIntensity * mix(parentColor, childColor, childColorBias);
		color.a = 1.0;
	}

}