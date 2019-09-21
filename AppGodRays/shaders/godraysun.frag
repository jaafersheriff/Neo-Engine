in vec2 fragTex;

uniform vec3 center;

out vec4 color;

void main() {
    // [0,1]
    vec2 uv = fragTex;
    // [-1,1]
    uv = (uv - 0.5) * 2.0;
    float l = length(uv);
    color = vec4(vec3(1.0 - (3.14 * l * l)), 1.0);
}