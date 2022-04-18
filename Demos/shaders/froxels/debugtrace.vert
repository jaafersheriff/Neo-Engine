layout(location = 0) in vec3 vertPos;

void main() {
    gl_Position = vec4(vertPos * 2.0, 1.0);
}