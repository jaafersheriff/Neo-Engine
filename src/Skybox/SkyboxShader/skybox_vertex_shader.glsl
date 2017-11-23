#version 330

layout(location = 0) in vec3 vertexPos;

uniform mat4 P;
uniform mat4 V;

out vec3 textureCoords;

void main(void){
    gl_Position = P * V * vec4(vertexPos, 1.0); 
    textureCoords = vertexPos;
}
