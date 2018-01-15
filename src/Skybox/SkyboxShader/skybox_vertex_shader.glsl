#version 330

layout(location = 0) in vec3 vertexPos;

uniform mat4 P;
uniform mat4 V;

out vec3 textureCoords;

void main(void){
    mat4 skyV = V;
    skyV[3][0] = skyV[3][1] = skyV[3][2] = 0.0;
    gl_Position = P * skyV * vec4(vertexPos, 1.0); 
    textureCoords = vertexPos;
}
