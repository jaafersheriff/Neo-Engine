layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P, V, M;
uniform mat3 N;

out vec3 fragPos;
out vec3 fragNor;

void main() {
      vec4 wv = M * vec4(vertPos.xyz, 1.0);
      gl_Position = P * V * wv;
      fragPos = wv.xyz;
      fragNor = N * vertNor;
}
