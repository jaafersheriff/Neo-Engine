
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gDepth;

uniform mat4 invP, invV;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform float lightRadius;

uniform bool showLights;
uniform float showRadius;

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

vec3 reconstructWorldPos(vec3 coords) {
	vec3 ndc = coords * 2.0 - vec3(1.0);
	vec4 view = invP * vec4(ndc, 1.0);
	view.xyz /= view.w;
	vec4 world = invV * vec4(view.xyz, 1.0);
	return world.xyz;
}

void main() {
    /* Generate tex coords [0, 1] */
    vec2 fragTex = gl_FragCoord.xy / vec2(textureSize(gDepth, 0));
    
    /* Calculate fragment's world position */
    float depth = texture(gDepth, fragTex).r;
    vec3 fragPos = reconstructWorldPos(vec3(fragTex, depth));

    if (showLights) {
        float rayDist = raySphereIntersect(camPos, normalize(fragPos - camPos), lightPos, lightRadius * showRadius);
        if (rayDist > 0.0 && rayDist < length(fragPos - camPos)) {
            color = vec4(lightCol, 1.f);
            return;
        }
    }

    /* Calculate attenuation 
     * Early discard if fragment is outside of light volume */
    vec3 lightDir = lightPos - fragPos;
    float lightDist = length(lightDir);
    float attFactor = 1.f - clamp((lightDist / lightRadius) * (lightDist / lightRadius), 0.f, 1.f);
    if (attFactor == 0.f) {
        discard;
    }

    /* Retrieve remaining data from gbuffer */
    vec3 fragNor = texture(gNormal, fragTex).rgb * 2.f - vec3(1.f);
    vec3 albedo = texture(gDiffuse, fragTex).rgb;
 
    vec3 L = normalize(lightDir);
    vec3 V = normalize(camPos - fragPos);
    vec3 N = normalize(fragNor);
    vec3 H = normalize(L + V);
    float lambert = clamp(dot(L, N), 0.f, 1.f);
    vec3 diffuseContrib = lightCol * lambert * attFactor;
    float s = pow(clamp(dot(H, N), 0.f, 1.f), 33.f);
    vec3 specularContrib = lightCol * s * attFactor * 0.33f;
    color.a = 1.f;
    color.rgb = diffuseContrib * albedo + specularContrib;
}