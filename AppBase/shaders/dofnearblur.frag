
in vec4 fragTex;

uniform sampler2D dofDown;

out vec4 color;

void main() {
  color = vec4(0);
  color += texture( dofDown, fragTex.xz );
  color += texture( dofDown, fragTex.yz );
  color += texture( dofDown, fragTex.xw );
  color += texture( dofDown, fragTex.yw );
  color /= 4;
}
