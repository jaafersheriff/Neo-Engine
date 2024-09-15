
in vec4 fragPos;

#ifdef BOUNDING_BOX
uniform vec3 bbMin;
uniform vec3 bbMax;
layout(binding = 0) uniform sampler2D hiZ;
#endif

out vec4 color;

void main() {
#ifndef BOUNDING_BOX
	color = vec4(1, 0, 0, 1);
	return;
#endif

	color = vec4(gl_FragCoord.zzz, 1.0);
}

