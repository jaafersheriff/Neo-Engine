
in vec2 fragTex;

uniform sampler2D dofDown;
uniform ivec2 frameSize;
uniform int blurSize;

out vec4 color;

void main() {
  color = vec4(0);

  // TODO - use a uniform for blur scale
  for(int x = -blurSize; x <= blurSize; x++) {
    for(int y = -blurSize; y <= blurSize; y++) {
        color += texture(dofDown, fragTex + vec2(x, y) / frameSize);
    }
  }

  color /= (blurSize * 2 + 1);
}
