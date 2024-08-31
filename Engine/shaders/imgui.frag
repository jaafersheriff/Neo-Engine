in vec2 fragTex;
in vec4 fragCol;
        
layout(binding = 0) uniform sampler2D Texture;

out vec4 color;

void main() {
    //color = fragCol * texture(Texture, fragTex);
    color = vec4(1,1,1,texture(Texture, fragTex).a);
}
