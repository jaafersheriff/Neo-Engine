layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec3[] gVelocity;
in float[] gIntensity;

out float fIntensity;

void main() {

	// base 
	gl_Position = P * V * gl_in[0].gl_Position;
	fIntensity = gIntensity[0];
	EmitVertex();

	// tail
	gl_Position = P * V * (gl_in[0].gl_Position + vec4(gVelocity[0] * 0.5, 0));
	fIntensity = gIntensity[0];
	EmitVertex();

	EndPrimitive();
}  