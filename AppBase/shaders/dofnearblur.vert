layout (location = 0) in vec3 vertPos;
layout (location = 2) in vec2 vertTex;

uniform vec2 invRenderTargetSize;

out vec4 fragTex;

void main() { 
    gl_Position = vec4(2 * vertPos, 1); 

    const vec4 halfPixel = { -0.5, 0.5, -0.5, 0.5 };
    fragTex = vertTex.xxyy + halfPixel * vec4(invRenderTargetSize, 0.0, 0.0);
}
