#include "phong.glsl"

layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 1) uniform sampler2D gSpecular;
layout(binding = 2) uniform sampler2D gWorld;
layout(binding = 3) uniform sampler2D gNormal;

uniform vec2 resolution;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform float lightRadius;

#ifdef SHOW_LIGHTS
uniform float debugRadius;
#endif

out vec4 color;

float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr)
{
	// https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
	// - r0: ray origin
	// - rd: normalized ray direction
	// - s0: sphere center
	// - sr: sphere radius
	// - Returns distance from r0 to first intersecion with sphere,
	//   or -1.0 if no intersection.
	float a = dot(rd, rd);
	vec3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sr * sr);
	if (b*b - 4.0*a*c < 0.0)
	{
		return -1.0;
	}
	return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}

void main() {
    /* Generate tex coords [0, 1] */
    vec2 fragTex = gl_FragCoord.xy / resolution;
    
    vec3 fragPos = texture(gWorld, fragTex).rgb;
    vec3 fragNor = normalize(texture(gNormal, fragTex).rgb * 2.f - vec3(1.f));

#ifdef SHOW_LIGHTS
    float rayDist = raySphereIntersect(camPos, normalize(fragPos - camPos), lightPos, debugRadius);
    if (rayDist > 0.0 && rayDist < length(fragPos - camPos)) {
        color = vec4(lightCol, 1.f);
        return;
    }
#endif

    /* Calculate attenuation 
     * Early discard if fragment is outside of light volume */
    vec3 lightDir = lightPos - fragPos;
    float lightDist = length(lightDir);
    float attFactor = 1.f - clamp((lightDist / lightRadius) * (lightDist / lightRadius), 0.f, 1.f);
    if (attFactor == 0.f) {
        discard;
    }

    /* Retrieve remaining data from gbuffer */
    vec3 albedo = texture(gAlbedo, fragTex).rgb;
    vec4 specularShine = texture(gSpecular, fragTex);
 
    vec3 L = normalize(lightDir);
    vec3 V = normalize(camPos - fragPos);
    vec3 N = fragNor;

    color.rgb = getPhong(V, N, L, albedo * 0.2, albedo, specularShine.rgb, specularShine.a, lightCol, attFactor);
    color.rgb *= attFactor * attFactor;
    color.a = 1.0;
}