
#include "postprocess.glsl"

in vec2 fragColorTex0;
in vec2 fragColorTex1;
in vec2 fragDepthTex0;
in vec2 fragDepthTex1;
in vec2 fragDepthTex2;
in vec2 fragDepthTex3;

uniform vec2 dofEqWorld; 
uniform vec2 dofEqWeapon; 
uniform vec2 dofRowDelta;  // vec2( 0, 0.25 / renderTargetHeight )    

out vec4 outColor;

// DofDown
void main() {
    mediump vec3 color;
    mediump float maxCoc;
    vec4 depth;
    mediump vec4 viewCoc;
    mediump vec4 sceneCoc;
    mediump vec4 curCoc;
    mediump vec4 coc;
    vec2 rowOfs[4];
    
    // "rowOfs" reduces how many moves PS2.0 uses to emulate swizzling.   
    rowOfs[0] = vec2(0.0);
    rowOfs[1] = dofRowDelta.xy;
    rowOfs[2] = dofRowDelta.xy * 2;
    rowOfs[3] = dofRowDelta.xy * 3;
    
    // Use bilinear filtering to average 4 color samples for free.   
    color = vec3(0.0);
    color += texture( inputFBO, fragColorTex0.xy + rowOfs[0] ).rgb;
    color += texture( inputFBO, fragColorTex1.xy + rowOfs[0] ).rgb;
    color += texture( inputFBO, fragColorTex0.xy + rowOfs[2] ).rgb;
    color += texture( inputFBO, fragColorTex1.xy + rowOfs[2] ).rgb;
    color /= 4;

    // Process 4 samples at a time to use vector hardware efficiently.    
    // The CoC will be 1 if the depth is negative, so use "min" to pick    
    // between "sceneCoc" and "viewCoc".   
    depth[0] = texture( inputDepth, fragDepthTex0.xy + rowOfs[0] ).r;
    depth[1] = texture( inputDepth, fragDepthTex1.xy + rowOfs[0] ).r;
    depth[2] = texture( inputDepth, fragDepthTex2.xy + rowOfs[0] ).r;
    depth[3] = texture( inputDepth, fragDepthTex3.xy + rowOfs[0] ).r;
    depth = depth * 0.5 + 0.5;
    viewCoc = clamp( dofEqWeapon.x * -depth + dofEqWeapon.y, 0.0, 1.0 );
    sceneCoc = clamp( dofEqWorld.x * depth + dofEqWorld.y, 0.0, 1.0 );
    curCoc = min( viewCoc, sceneCoc );
    coc = curCoc;
    depth[0] = texture( inputDepth, fragDepthTex0.xy + rowOfs[1] ).r;
    depth[1] = texture( inputDepth, fragDepthTex1.xy + rowOfs[1] ).r;
    depth[2] = texture( inputDepth, fragDepthTex2.xy + rowOfs[1] ).r;
    depth[3] = texture( inputDepth, fragDepthTex3.xy + rowOfs[1] ).r;
    depth = depth * 0.5 + 0.5;
    viewCoc = clamp( dofEqWeapon.x * -depth + dofEqWeapon.y, 0.0, 1.0 );
    sceneCoc = clamp( dofEqWorld.x * depth + dofEqWorld.y, 0.0, 1.0 );
    curCoc = min( viewCoc, sceneCoc );
    coc = max( coc, curCoc );
    depth[0] = texture( inputDepth, fragDepthTex0.xy + rowOfs[2] ).r;
    depth[1] = texture( inputDepth, fragDepthTex1.xy + rowOfs[2] ).r;
    depth[2] = texture( inputDepth, fragDepthTex2.xy + rowOfs[2] ).r;
    depth[3] = texture( inputDepth, fragDepthTex3.xy + rowOfs[2] ).r;
    depth = depth * 0.5 + 0.5;
    viewCoc = clamp( dofEqWeapon.x * -depth + dofEqWeapon.y, 0.0, 1.0 );
    sceneCoc = clamp( dofEqWorld.x * depth + dofEqWorld.y, 0.0, 1.0 );
    curCoc = min( viewCoc, sceneCoc );
    coc = max( coc, curCoc );
    depth[0] = texture( inputDepth, fragDepthTex0.xy + rowOfs[3] ).r;
    depth[1] = texture( inputDepth, fragDepthTex1.xy + rowOfs[3] ).r;
    depth[2] = texture( inputDepth, fragDepthTex2.xy + rowOfs[3] ).r;
    depth[3] = texture( inputDepth, fragDepthTex3.xy + rowOfs[3] ).r;
    depth = depth * 0.5 + 0.5;
    viewCoc = clamp( dofEqWeapon.x * -depth + dofEqWeapon.y, 0.0, 1.0 );
    sceneCoc = clamp( dofEqWorld.x * depth + dofEqWorld.y, 0.0, 1.0 );
    curCoc = min( viewCoc, sceneCoc );
    coc = max( coc, curCoc );
    maxCoc = max( max( coc[0], coc[1] ), max( coc[2], coc[3] ) );
    outColor = vec4( color, maxCoc );
}