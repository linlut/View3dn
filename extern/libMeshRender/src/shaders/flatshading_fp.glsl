#if !defined(__STDC__) && !defined(__cplusplus)

#version 330 compatibility 
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

#include "common_uniforms.glsl"
#include "normal_decompression.glsl"
#include "shading.glsl"

uniform sampler2DRect texElmNormal; //Element normal texture;


noperspective in vec3 dist;

void main(void)
{
    vec2 elmAttribMinMax = getAttributeMinMax();
    vec4 frontColor = vec4(getFaceFrontColor(), 1.0);
    vec4 backColor = vec4(getFaceBackColor(), 1.0);
    vec4 vertexPos = vec4(gl_TexCoord[0].xyz, 1.0);

    int pid = gl_PrimitiveID;
    int texwidth = textureSize(texElmNormal).x;
    ivec2 texcoord2 = ivec2(pid % texwidth, pid / texwidth);
    vec2 texel = texelFetch(texElmNormal, texcoord2).xy;
    vec3 norm = decompress_normal(floatBitsToUint(texel.x)); 
    
    vec4 facecolor = frontColor;
    if (!gl_FrontFacing) facecolor = backColor;
    vec4 fragcolor = facecolor;
    fragcolor.xyz = shadeVertex(vertexPos, norm, facecolor.xyz);
    
    int s = int(4.0 * texel.y / ( elmAttribMinMax.y - elmAttribMinMax.x));
    if (s <= 0){
        fragcolor.x += 0.35;
    }
    else if (s == 1){
        fragcolor.y += 0.35;
    }
    else if (s == 2){
        fragcolor.z += 0.35;
    }
    else{
        fragcolor.xy += 0.35;
    }
  
    fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);
    gl_FragColor = fragcolor;
    
}

#endif