#if !defined(__STDC__) && !defined(__cplusplus)

#version 330 compatibility 
#extension GL_EXT_gpu_shader4 : enable

void main(void)
{
    vec4 pos = vec4(gl_Vertex.xyz, 1.0);

    gl_Position = gl_ModelViewProjectionMatrix * pos;    
    
    gl_TexCoord[0] = pos;
}

#endif