#if !defined(__STDC__) && !defined(__cplusplus)

#version 330 compatibility 
#extension GL_EXT_gpu_shader4 : enable

#include "common_uniforms.glsl"
#include "normal_decompression.glsl"
#include "shading.glsl"

void main(void)
{
    vec4 frontColor = vec4(getFaceFrontColor(), 1.0);
    vec4 backColor = vec4(getFaceBackColor(), 1.0);

    vec4 pos = vec4(gl_Vertex.xyz, 1.0);
    vec3 norm = decompress_normal(floatBitsToUint(gl_Vertex.w));

    vec3 fragFrontColor = shadeVertex(gl_Vertex, norm, frontColor.xyz);
    vec3 fragBackColor = shadeVertex(gl_Vertex, norm, backColor.xyz);
    
    gl_Position = gl_ModelViewProjectionMatrix * pos;    
    gl_FrontColor = vec4(fragFrontColor, 1.0);
    gl_FrontSecondaryColor = vec4(fragBackColor, 1.0);
    gl_TexCoord[0] = pos;
    gl_TexCoord[1].xyz = norm;
}

#endif
