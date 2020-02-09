
in vec2 fragTex;

uniform sampler2D dofDown;
uniform ivec2 frameSize;

out vec4 color;

void main() {
  color = vec4(0);

  // TODO - use a uniform for blur scale
  for(int x = -1; x <= 1; x++) {
    for(int y = -1; y <= 1; y++) {
        color += texture(dofDown, fragTex + vec2(x, y) / frameSize);
    }
  }

  color /= 9;
}
