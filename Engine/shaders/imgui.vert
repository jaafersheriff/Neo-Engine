layout(location = 0) in vec2 vertPos;
layout(location = 1) in vec4 vertCol;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;

out vec2 fragTex;
out vec4 fragCol;

void main() {
	fragTex = vertTex;
	fragCol = vertCol;

	gl_Position = P * vec4(vertPos.xy, 0.0, 1.0);
}