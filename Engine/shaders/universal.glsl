
#define PI 3.141592653589


#define saturate(_x) clamp(_x, 0.0, 1.0)
#define mul(_a, _b) ( (_a) * (_b) )

vec4 srgbToLinear(vec4 srgb) {
	return vec4(pow(srgb.xyz, vec3(2.2)), srgb.w);;
}
vec4 linearToSrgb(vec4 srgb) {
	return vec4(pow(srgb.xyz, vec3(1/2.2)), srgb.w);;
}

