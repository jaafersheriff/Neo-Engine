layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float spriteSize;

out vec2 fragTex;

void main() {

	// base 
	gl_Position = gl_in[0].gl_Position;
	fragTex = vec2(0,0);
	EmitVertex();

	// right 
	gl_Position = gl_in[0].gl_Position + vec4(vec3(1,0,0) * spriteSize, 0.0);
	fragTex = vec2(1,0);
	EmitVertex();
	
	// up 
	gl_Position = gl_in[0].gl_Position + vec4(vec3(0,1,0) * spriteSize, 0.0);
	fragTex = vec2(0,1);
	EmitVertex();
	
	// up right
	gl_Position = gl_in[0].gl_Position + vec4(vec3(1,1,0) * spriteSize, 0.0);
	fragTex = vec2(1,1);
	EmitVertex();

	EndPrimitive();
}  