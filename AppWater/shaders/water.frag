in vec4 fragPos;
in vec4 fragTex;

out vec4 color;

void main() {
    color = vec4(fragTex.xy, 0.0, 1.0);
} 