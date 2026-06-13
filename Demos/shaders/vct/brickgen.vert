// TODO - this could jsut be model.vert	
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

out vec4 geomPos;
out vec3 geomNor;
out vec2 geomTex;

void main() {
	geomPos = M * vec4(vertPos, 1.0);
	geomNor = N * vertNor;
	geomTex = vertTex;
	gl_Position = P * V * geomPos;
}
