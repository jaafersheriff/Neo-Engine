
in vec3 fragColor;

out vec4 color;

void main() {
    color = vec4(fragColor, 1.0);
    // color = vec4(vec3(1,1,0), 1.0);
}  