
in vec4 fragCol;
in vec2 fragTex;

out vec4 color;

void main() {
    // Quick fall-off computation
    // float r = length(fragTex*2.0-1.0)*3.0;
    // float i = exp(-r*r);
    // if (i < 0.01) discard;

    // color = vec4(fragCol.rgb, i);
    color = vec4(1.0);
}