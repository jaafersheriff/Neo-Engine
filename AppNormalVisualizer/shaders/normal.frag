
in vec3 fragColor;

out vec4 color;

void main() {
    color = vec4(normalize(fragColor), 1.0);
}  