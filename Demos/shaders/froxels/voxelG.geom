layout (points) in;
layout (points, max_vertices = 1) out;

uniform mat4 P;
uniform mat4 V;

in vec4 pos[];
in vec4 col[];

out vec3 fragColor;

void main() {
    // if(col[0].a > 0) {
        fragColor = col[0].rgb;
        gl_Position = P * V * vec4(pos[0].xyz, 1.0);
        EmitVertex();

        EndPrimitive();
    // }
}  
