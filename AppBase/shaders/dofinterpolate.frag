
#include "postprocess.glsl"

uniform sampler2D dofBlur;    // Output of SmallBlurPS()        
uniform sampler2D dofDown;    // Blurred output of DofDownsample()        
uniform vec2 invRenderTargetSize;
uniform vec4 dofLerpScale;
uniform vec4 dofLerpBias;
uniform vec3 dofEqFar;

out vec4 color;

vec4 tex2Doffset( sampler2D s, vec2 tc, vec2 offset ) {     
    return texture( s, tc + offset * invRenderTargetSize );
} 

mediump vec3 GetSmallBlurSample( vec2 texCoords ) {     
    mediump vec3 sum = vec3(0.0);
    const mediump float weight = 4.0 / 17;

    // Unblurred sample done by alpha blending     
    sum = weight * tex2Doffset( inputFBO, texCoords, vec2( 0.5, -1.5 )).rgb;
    sum = weight * tex2Doffset( inputFBO, texCoords, vec2(-1.5, -0.5 )).rgb;
    sum = weight * tex2Doffset( inputFBO, texCoords, vec2(-0.5,  1.5 )).rgb;
    sum = weight * tex2Doffset( inputFBO, texCoords, vec2( 1.5,  0.5 )).rgb;
    return sum;
} 

vec4 InterpolateDof( mediump vec3 small, mediump vec3 med, mediump vec3 large, mediump float t ) {     
    // Efficiently calculate the cross-blend weights for each sample.        
    // Let the unblurred sample to small blur fade happen over distance        
    // d0, the small to medium blur over distance d1, and the medium to        
    // large blur over distance d2, where 
    // d0 + d1 + d2 = 1
    mediump vec4 weights = clamp( t * dofLerpScale + dofLerpBias, 0.0, 1.0);
    weights.yz = min( weights.yz, 1 - weights.xy );

    // Unblurred sample with weight "weights.x" done by alpha blending     
    mediump vec3 color = weights.y * small + weights.z * med + weights.w * large;
    float alpha = dot( weights.yzw, vec3( 16.0 / 17, 1.0, 1.0 ) );
    return vec4( color, alpha );
} 

void main() {
    mediump vec3 small = GetSmallBlurSample( fragTex );
    mediump vec4 med = texture( dofBlur, fragTex );
    mediump vec3 large = texture( dofDown, fragTex ).rgb;
    mediump float depth;
    mediump float nearCoc;
    mediump float farCoc;
    mediump float coc;
    nearCoc = med.a;
    depth = texture( inputDepth, fragTex ).r;
    // if ( depth > 1.0e6 )     {         
    //         coc = nearCoc;
    //         // We don't want to blur the sky.     
    // }     
    // else {         
        // dofEqFar.x and dofEqFar.y specify the linear ramp to convert        
        // to depth for the distant out-of-focus region.        
        // dofEqFar.z is the ratio of the far to the near blur radius.         
        farCoc = clamp( dofEqFar.x * depth + dofEqFar.y, 0.0, 1.0 );
        coc = max( nearCoc, farCoc * dofEqFar.z );
    // }     
    
    color = InterpolateDof( small, med.rgb, large, coc );
}
