#include "postprocess.glsl"

uniform sampler2D dofblur;
uniform sampler2D dofinfo;

uniform int poissonSize;
uniform sampler1D poissonTex; // poisson-distributed positions on the unit circle

uniform vec2 scenePixelSize; // pixel size ( 1 / image resolution ) of full resolution image
uniform vec2 dofPixelSize; // pixel size of dofblur

uniform vec2 maxCoc; // max circle of confusion radius and diameter (in pixels)

uniform float radiusScale; // scale factor for max coc size on dofblur

out vec4 color;

void main() { 

    // convert depth into blur radius in pixels
    float depthCenter = texture(dofinfo, fragTex).r;
    float discRadius = abs(depthCenter * maxCoc.y - maxCoc.x);
    float discRadiusBlur = discRadius * radiusScale; // compute radius on dofblur

    vec4 centerOut = vec4(0.0);
    for(float i = 0.0; i < 1.0; i += 1.0 / poissonSize) {
        // compute tap texture coordinates
        vec2 poisson = texture(poissonTex, i).rg;
        vec2 coordBlur  = fragTex + (dofPixelSize * poisson * discRadiusBlur);
        vec2 coordScene = fragTex + (scenePixelSize * poisson * discRadius);

        // get scene tap
        vec4 blurTap = vec4(texture(inputFBO, coordBlur).rgb, texture(dofinfo, coordBlur).r);
        vec4 sceneTap = vec4(texture(inputFBO, coordScene).rgb, texture(dofinfo, coordScene).r);

        // mix low and high-res taps based on blurrines
        vec4 tapBlur = vec4(abs(sceneTap.a * 2.0 - 1.0)); // put bluriness into [0, 1] 
        vec4 tap = mix(sceneTap, blurTap, tapBlur);

        // smart blur - ignore taps that are closer than the center tap and in focus
        tap.a = (tap.a >= depthCenter) ? 1.0 : abs(tap.a * 2.0 - 1.0);

        centerOut.rgb += tap.rgb * tap.a;
        centerOut.a += tap.a;
    }

    color = vec4(centerOut.rgb / centerOut.a, 1.0);
}   
