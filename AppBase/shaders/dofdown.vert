layout (location = 0) in vec3 vertPos;
layout (location = 2) in vec2 vertTex;

uniform vec2 invRenderTargetSize;

out vec2 fragTex;
out vec2 fragColorTex0;
out vec2 fragColorTex1;
out vec2 fragDepthTex0;
out vec2 fragDepthTex1;
out vec2 fragDepthTex2;
out vec2 fragDepthTex3;

void main() { 
    gl_Position = vec4(2 * vertPos, 1); 
    
    fragTex = vertTex; 
    fragColorTex0 = vertTex + vec2( -1.0, -1.0 ) * invRenderTargetSize;   
    fragColorTex1 = vertTex + vec2(  1.0, -1.0 ) * invRenderTargetSize;
    fragDepthTex0 = vertTex + vec2( -1.5, -1.5 ) * invRenderTargetSize;
    fragDepthTex1 = vertTex + vec2( -0.5, -1.5 ) * invRenderTargetSize;
    fragDepthTex2 = vertTex + vec2(  0.5, -1.5 ) * invRenderTargetSize;
    fragDepthTex3 = vertTex + vec2(  1.5, -1.5 ) * invRenderTargetSize;
}