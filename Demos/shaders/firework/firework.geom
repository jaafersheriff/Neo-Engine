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
	float scale = saturate(gIntensity[0]);
	gl_Position = P * V * (gl_in[0].gl_Position + vec4(normalize(gVelocity[0]) * scale, 0));
	fIntensity = gIntensity[0] / 9;
	EmitVertex();

	EndPrimitive();
}  