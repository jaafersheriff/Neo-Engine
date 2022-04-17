#include "phong.glsl"
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;


layout(rgba32f, binding = 0) uniform restrict writeonly image3D volume;
uniform vec2 bufferSize;
uniform float camNear;
uniform float camFar;

uniform sampler2D diffuseMap;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shine;
uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

out vec4 color;

float linearizeDepth(float d,float zNear,float zFar) {
	float z_n = 2.0 * d - 1.0;
    float z_r = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	return z_r;
}

void writevoxel(vec4 color) {
	ivec3 dims = imageSize(volume) - 1;

	vec2 xy = gl_FragCoord.xy;
	xy /= bufferSize;
	float depth = gl_FragCoord.z;

	float ldepth = linearizeDepth(gl_FragCoord.z, camNear, camFar) / camFar;

	ivec3 volCoords = ivec3(xy * dims.xy, ldepth * dims.z);
	volCoords = clamp(volCoords, ivec3(0), dims);

	imageStore(volume, volCoords, color);
}
void main() {
    vec4 albedo = texture(diffuseMap, fragTex);
    albedo.rgb += diffuseColor;
    alphaDiscard(albedo.a);
    color.rgb = albedo.rgb * ambientColor + 
                getPhong(fragNor, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, albedo.rgb, specularColor, shine);
    color.a = albedo.a;
    writevoxel(color);
}
