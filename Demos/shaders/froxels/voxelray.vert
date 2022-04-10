layout(location = 0) in vec3 vertPos;
uniform mat4 P, V, M;
uniform mat4 mockP, mockV;

out vec4 mock_gl_Position;
void main() {
    gl_Position = P * V * M * vec4(vertPos, 1.0);
    mock_gl_Position = mockP * mockV * M * vec4(vertPos, 1.0);

}
