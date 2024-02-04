#include "postprocess.glsl"

uniform sampler2D godray;
uniform float exposure;
uniform vec3 sunColor;

out vec4 color;

void main() {

	color = texture(inputFBO, fragTex) + vec4(texture(godray, fragTex).r) * vec4(sunColor, 1.0) * exposure;
	
}