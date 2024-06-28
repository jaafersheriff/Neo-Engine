

float luminance(vec3 v) {
    return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec4 srgbToLinear(vec4 srgb) {
	return vec4(pow(srgb.xyz, vec3(2.2)), srgb.w);;
}
vec4 linearToSrgb(vec4 srgb) {
	return vec4(pow(srgb.xyz, vec3(1/2.2)), srgb.w);;
}

