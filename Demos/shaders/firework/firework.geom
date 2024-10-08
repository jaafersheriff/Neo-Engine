layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float parentLength;
uniform float childLength;

in vec4[] gVelocity;
in float[] gIntensity;

out float fIntensity;
out float fParent;

void main() {

	// base 
	gl_Position = P * V * gl_in[0].gl_Position;
	fParent = gVelocity[0].a;
	if (fParent > 0.9) {
		fIntensity = gIntensity[0];
	}
	else {
		fIntensity = 0.0;
	}

	EmitVertex();

	// tail
	float scale = saturate(gIntensity[0]);
	fParent = gVelocity[0].a;
	 if (fParent > 0.9) {
	 	scale *= parentLength;
		fIntensity = 0.0;
	 }
	 else {
		 scale *= childLength;
		 fIntensity = gIntensity[0];
	 }
	gl_Position = P * V * (gl_in[0].gl_Position - vec4(normalize(gVelocity[0].xyz) * scale, 0));
	EmitVertex();

	EndPrimitive();
}  