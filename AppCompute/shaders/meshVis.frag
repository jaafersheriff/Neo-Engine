in vec3 outPos;

out vec4 color;

uniform bool wf;

void main() {
    if (wf) {
        color = vec4(1);
    }
    else {
        color = vec4(outPos,1);
    }
}