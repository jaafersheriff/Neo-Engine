layout(location = 0) in vec4 vertPos;

uniform mat4 P, V, M;

out vec3 outPos;

void main()
{
      gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);
      outPos = vertPos.xyz;
}