layout (points) in;
layout (line_strip, max_vertices = 2) out;

//uniform mat4 P;
//uniform mat4 V;

//in vec3 gPos;
//in float gDecay;
//in vec3 gVelocity;

//out float fDecay;

void main() {

	// base 
	//gl_Position = gl_in[0].gl_Position;
	//fDecay = gDecay;
	EmitVertex();

	// tail
	//gl_Position = P * V * vec4(gPos + gVelocity, 1.0);
	//fDecay = gDecay;
	EmitVertex();

	EndPrimitive();
}  