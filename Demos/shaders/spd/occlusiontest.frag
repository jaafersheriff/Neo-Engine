
in vec4 fragPos;

flat in int isVisible;
in vec3 debugColor;

out vec4 color;


void main() {
	if (isVisible == 1) {
		color = vec4(1);
	}
	else {
		color = vec4(1,0,0, 1.0);
		//discard;
	}

	//color = vec4(debugColor, 1.0);

}

