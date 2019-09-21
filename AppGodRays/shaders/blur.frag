
in vec2 fragTex;

uniform sampler2D godray;
uniform float blurSteps;

uniform vec3 sunPos;
uniform mat4 P;
uniform mat4 V;

uniform float decay;
uniform float density;
uniform float weight;

out vec4 outcolor;

void main() {

    // sun position in screen space
    vec4 clipspace = P * V * vec4(sunPos, 1.0);
    vec3 ndcspace = clipspace.xyz / clipspace.w;
    vec2 sspace = clamp((ndcspace.xy + 1.0) / 2.0, 0.0, 1.0);
    
    float exposure=0.2;
    
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

