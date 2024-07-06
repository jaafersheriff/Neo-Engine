#include "color.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;

#ifdef AUTO_EXPOSURE
layout(binding = 1) uniform sampler2D averageLuminance;
#endif

out vec4 color;

vec3 convertRGB2Yxy(vec3 _rgb) {
    // Reference:
    // RGB/XYZ Matrices
    // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
    vec3 xyz;
    xyz.x = dot(vec3(0.4124564f, 0.3575761f, 0.1804375f), _rgb);
    xyz.y = dot(vec3(0.2126729f, 0.7151522f, 0.0721750f), _rgb);
    xyz.z = dot(vec3(0.0193339f, 0.1191920f, 0.9503041f), _rgb);

    // Reference:
    // http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
    float inv = 1.0f / dot(xyz, vec3(1.0f));
    return vec3(xyz.y, xyz.x * inv, xyz.y * inv);
}

vec3 convertYxy2RGB(vec3 _Yxy) {
    // Reference:
    // http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
    vec3 xyz;
    xyz.x = _Yxy.x * _Yxy.y / _Yxy.z;
    xyz.y = _Yxy.x;
    xyz.z = _Yxy.x * (1.0f - _Yxy.y - _Yxy.z) / _Yxy.z;

    vec3 rgb;
    rgb.x = dot(vec3( 3.2404542f, -1.5371385f, -0.4985314f), xyz);
    rgb.y = dot(vec3(-0.9692660f,  1.8760108f,  0.0415560f), xyz);
    rgb.z = dot(vec3( 0.0556434f, -0.2040259f,  1.0572252f), xyz);
    return rgb;
}

float Uncharted2(float x) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02;
    const float F = 0.30f;
    const float W = 11.2f;

    x =  ((x*(A*x + C*B) + D*E) / (x*(A*x + B) + D*F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    
    return x / white;
}

void main() {

	vec3 v = texture(inputTexture, fragTex).rgb;
#ifdef AUTO_EXPOSURE
	float avgLum = texture(averageLuminance, vec2(0, 0)).r;
	vec3 Yxy = convertRGB2Yxy(v);

	float lp = Yxy.x / (9.6 * avgLum + 0.0001);
    Yxy.x = Uncharted2(lp);

	v = convertYxy2RGB(Yxy);
#endif

	color = linearToSrgb(vec4(v, 1.0));
}