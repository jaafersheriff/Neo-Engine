
layout(location = 0) in vec4 vertPos;

uniform mat4 P, V, M;

void main() {

    mat4 v = V;
    v[3][0] = v[3][1] = v[3][2] = 0.0;
    v[3][3] = 1.0;
    gl_Position = P * V * inverse(v) * vec4(vertPos.xyz, 1.0);
}
