layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec3[] gPos;
in vec3[] gVelocity;
in float[] gIntensity;

out float fIntensity;

void main() {

	// base 
	//gl_Position = gl_in[0].gl_Position;
	//fIntensity = gIntensity[0];
	EmitVertex();

	// tail
	//gl_Position = P * V * M * vec4(gPos[0] + gVelocity[0], 1.0);
	//fIntensity = gIntensity[0];
	EmitVertex();

	EndPrimitive();
}  