layout(location = 0) in vec3 vertPos;

out vec2 fragTex;
void main() {
    // quad is -0.5, 0.5
    fragTex = vertPos.xy + 0.5;
    gl_Position = vec4(vertPos * 2.0, 1.0);
}