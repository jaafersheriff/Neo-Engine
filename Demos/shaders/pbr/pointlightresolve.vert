layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec4 fragPos;

void main() {
	fragPos = M * vec4(vertPos, 1.0);
	gl_Position = P * V * fragPos;

	// Lights can be so big they go past the far plane and cause fragments to get culled even though the provide light
	// Clamp to the far plane
	gl_Position.z = min(gl_Position.z, gl_Position.w);

}
