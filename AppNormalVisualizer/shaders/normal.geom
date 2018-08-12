#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform float magnitude;

in vec3 fragNor[];

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = gl_in[i].gl_Position + vec4(fragNor[i], 0.0) * magnitude;
        EmitVertex();
        EndPrimitive();
    }
}  
