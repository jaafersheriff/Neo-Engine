
in vec4 fragPos;

#ifdef BOUNDING_BOX
uniform vec3 bbMin;
uniform vec3 bbMax;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
layout(binding = 0) uniform sampler2D hiZ;
uniform int hiZMips;
uniform vec2 hiZDimension;
#endif

out vec4 color;

bool _singleMipVisible(int mip) {
	return gl_FragCoord.z <= textureLod(hiZ, gl_FragCoord.xy / hiZDimension, mip).r + 1e-2;
}

bool isVisible() {
	int currentMip = hiZMips;
	while (currentMip > 0) {
		if (_singleMipVisible(currentMip)) {
			currentMip--;
		}
		else {
			return false;
		}
	}
	return true;
}

void main() {
#ifndef BOUNDING_BOX
	color = vec4(1, 0, 0, 1);
	return;
#endif

	//color = vec4(gl_FragCoord.zzz, 1.0);
	bool visible = isVisible();
	if (visible) {
		color = vec4(1);
	}
	else {
		color = vec4(1,0,0, 1.0);
	}
}

