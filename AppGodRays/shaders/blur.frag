
in vec2 fragTex;

uniform float blurAmount;
uniform sampler2D godray;

out vec4 color;

void main() {

    color = texture(godray, fragTex + blurAmount);

}

