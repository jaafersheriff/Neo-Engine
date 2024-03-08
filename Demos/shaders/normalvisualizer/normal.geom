layout (triangles) in;
layout (line_strip, max_vertices = 27) out;

uniform float magnitude;

in vec3 geomNor[];
in vec3 fragNor[];

out vec3 fragColor;

void main() {
	for (int i = 0; i < 3; i++) {
		// Normal
		fragColor = vec3(0, 1, 0);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		fragColor = vec3(0, 1, 0);
		gl_Position = gl_in[i].gl_Position + vec4(geomNor[i], 0.0) * magnitude;
		EmitVertex();

		fragColor = vec3(0, 1, 0);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		// Tangent
		fragColor = vec3(1, 0, 0);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		fragColor = vec3(1, 0, 0);
		gl_Position = gl_in[i].gl_Position + vec4(1,0,0, 0) * magnitude;
		EmitVertex();

		fragColor = vec3(1, 0, 0);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		// BiTangent
		fragColor = vec3(0, 0, 1);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		fragColor = vec3(0, 0, 1);
		gl_Position = gl_in[i].gl_Position + vec4(0,1,0, 0) * magnitude;
		EmitVertex();

		fragColor = vec3(0, 0, 1);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();



		EndPrimitive();
	}
}  
