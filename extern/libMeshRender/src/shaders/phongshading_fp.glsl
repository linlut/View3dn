#if !defined(__STDC__) && !defined(__cplusplus)
#version 330 compatibility 
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

#include "common_uniforms.glsl"
#include "shading.glsl"

noperspective in vec3 dist;

void main(void)
{
    vec4 vertexPos = gl_TexCoord[0];
    vec3 vertexNorm = gl_TexCoord[1].xyz;

    vec4 facecolor = gl_Color;
    if (!gl_FrontFacing){
        facecolor.xyz = gl_SecondaryColor.xyz;
    }
        
    vec4 fragcolor = facecolor;
    fragcolor.xyz = shadeVertex(vertexPos, vertexNorm, facecolor.xyz);
    fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);

    gl_FragColor = fragcolor;
    
}

#endif
