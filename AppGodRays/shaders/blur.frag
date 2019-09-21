
in vec2 fragTex;

uniform sampler2D godray;
uniform float blurDistance;
uniform float blurSteps;

uniform vec3 sunPos;
uniform mat4 P;
uniform mat4 V;

out vec4 outcolor;

void main() {

    // sun position in screen space
    // vec4 clipspace = P * V * vec4(sunPos, 1.0);
    // vec3 ndcspace = clipspace.xyz / clipspace.w;
    // vec2 sspace = clamp((ndcspace.xy + 1.0) / 2.0, 0.0, 1.0);

    // vec2 dir = sspace - fragTex; 
    // float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
    // dir = dir/dist; 

    // vec4 color = texture2D(godray, fragTex); 
    // vec4 sum = color;

    // for (float i = -blurDistance; i <= blurDistance; i += 2.0 * blurDistance / blurSteps) 
    //     sum += texture2D( godray, clamp(fragTex + dir * i, 0.0, 1.0) );

    // sum *= 1.0/11.0;
    // outcolor = sum;
 

// sun position in screen space
vec4 clipspace = P * V * vec4(sunPos, 1.0);
vec3 ndcspace = clipspace.xyz / clipspace.w;
vec2 sspace = clamp((ndcspace.xy + 1.0) / 2.0, 0.0, 1.0);

float decay=0.96815;
float exposure=0.2;
float density=0.926;
float weight=0.58767;

vec2 tc = fragTex;
vec2 dtc = density * (tc - sspace) / blurSteps;
outcolor = vec4(dtc, 0.0, 1.0);
float illdecay = 1.0;
vec4 color = texture(godray, tc.xy) * 0.4;
for(int i = 0; i < blurSteps; i++) {
tc -= dtc;
vec4 s = texture(godray, clamp(tc, 0.0, 1.0))*0.4;
s *= illdecay * weight;
color += s;
illdecay *= decay;
}
outcolor = color;

}

// /// Our light scattering pass texture
// uniform sampler2D UserMapSampler;
// /// Indicate where is the light source on the screen (2D position)
// uniform vec2 lightPositionOnScreen;
// void main()
// {
//  float decay=0.96815;
//  float exposure=0.2;
//  float density=0.926;
//  float weight=0.58767;
//  /// NUM_SAMPLES will describe the rays quality, you can play with
//  int NUM_SAMPLES = 100;
//  vec2 tc = gl_TexCoord[0].xy;
//  vec2 deltaTexCoord = (tc — lightPositionOnScreen.xy);
//  deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * density;
//  float illuminationDecay = 1.0;
//  vec4 color =texture2D(UserMapSampler, tc.xy)*0.4;
//  for(int i=0; i < NUM_SAMPLES ; i++)
//  {
//     tc -= deltaTexCoord;
//     vec4 sample = texture2D(UserMapSampler, tc)*0.4;
//     sample *= illuminationDecay * weight;
//     color += sample;
//     illuminationDecay *= decay;
//  }
//  gl_FragColor = color;
// }

