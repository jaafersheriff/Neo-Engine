layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

in vec4[] gVelocity;
in float[] gIntensity;

out float fIntensity;
out float fParent;

void main() {

	// base 
	gl_Position = P * V * gl_in[0].gl_Position;
	fIntensity = gIntensity[0];
	fParent = gVelocity[0].a;
	EmitVertex();

	// tail
	float scale = saturate(gIntensity[0]);
	fParent = gVelocity[0].a;
	if (fParent > 0.9) {
		scale *= 0.22;
	}
	else {
		scale *= 0.4;
	}
	gl_Position = P * V * (gl_in[0].gl_Position - vec4(normalize(gVelocity[0].xyz) * scale, 0));
	fIntensity = 0.0;
	EmitVertex();

	EndPrimitive();
}  