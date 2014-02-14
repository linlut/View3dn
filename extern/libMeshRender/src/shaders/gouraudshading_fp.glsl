#if !defined(__STDC__) && !defined(__cplusplus)
#version 330 compatibility 
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

#include "common_uniforms.glsl"

noperspective in vec3 dist;

void main(void)
{
    vec4 fragcolor = gl_Color; 
    if (!gl_FrontFacing){
        fragcolor.xyz = gl_SecondaryColor.xyz;
    }
        
    fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);
    gl_FragColor = fragcolor;
}

#endif
