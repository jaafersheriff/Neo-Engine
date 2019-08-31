
in vec2 fragTex;

uniform sampler2D fbo;

out vec4 color;

void main() {
    color.rgb = texture(fbo, fragTex).rgb;
    color.a = 1;
}