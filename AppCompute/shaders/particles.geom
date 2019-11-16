layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

float spriteSize = 0.2;

void main() {

    gl_Position = gl_in[0].gl_Position + vec4(normalize(vec3(-1,-1,0)) * spriteSize, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(normalize(vec3( 1,-1,0)) * spriteSize, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(normalize(vec3(-1, 1,0)) * spriteSize, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(normalize(vec3( 1, 1,0)) * spriteSize, 0.0);
    EmitVertex();

    EndPrimitive();
}  