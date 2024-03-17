layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform float magnitude;

in vec3 geomNor[];
in vec3 fragNor[];

out vec3 fragColor;

void main() {
	for (int i = 0; i < 3; i++) {
		fragColor = fragNor[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		fragColor = fragNor[i];
		gl_Position = gl_in[i].gl_Position + vec4(geomNor[i], 0.0) * magnitude;
		EmitVertex();

		EndPrimitive();
	}
}  
