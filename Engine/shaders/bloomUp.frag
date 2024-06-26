in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;
uniform float filterRadius;

out vec4 color;

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
void main() {
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(inputTexture, vec2(fragTex.x - x, fragTex.y + y)).rgb;
    vec3 b = texture(inputTexture, vec2(fragTex.x,     fragTex.y + y)).rgb;
    vec3 c = texture(inputTexture, vec2(fragTex.x + x, fragTex.y + y)).rgb;

    vec3 d = texture(inputTexture, vec2(fragTex.x - x, fragTex.y)).rgb;
    vec3 e = texture(inputTexture, vec2(fragTex.x,     fragTex.y)).rgb;
    vec3 f = texture(inputTexture, vec2(fragTex.x + x, fragTex.y)).rgb;

    vec3 g = texture(inputTexture, vec2(fragTex.x - x, fragTex.y - y)).rgb;
    vec3 h = texture(inputTexture, vec2(fragTex.x,     fragTex.y - y)).rgb;
    vec3 i = texture(inputTexture, vec2(fragTex.x + x, fragTex.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    vec3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    color = vec4(upsample, 1.0);
}