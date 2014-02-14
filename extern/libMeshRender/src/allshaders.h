//Code generated by glsl2c. User should make no changes on this file!
#pragma once
#include <string>
#include <map>
namespace MeshRender{
std::map<std::string, const char*> shadersMap;
inline int shaderSourceInit(){
    static const char* common_uniforms_glsl="uniform vec4 uniformsf[6];\nuniform ivec4 uniformsint[1];\n#define FACE_FRONT_COLOR_ROW 0\n#define FACE_BACK_COLOR_ROW 1\n#define LINE_COLOR_ROW 2\n#define SCREEN_SIZE_ROW 3\n#define ATTRIBUTE_MINMAX_ROW 4\n#if !defined(__STDC__) && !defined(__cplusplus)\nvec3 getFaceFrontColor(){\n int i = FACE_FRONT_COLOR_ROW;\n return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);\n}\nvec3 getFaceBackColor(){\n int i = FACE_BACK_COLOR_ROW;\n return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);\n}\nvec3 getLineColor(){\n int i = LINE_COLOR_ROW;\n return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);\n}\nfloat getLineWidth(){\n const int i = LINE_COLOR_ROW;\n return uniformsf[i].w;\n}\nvec2 getScreenSize(){\n const int i = SCREEN_SIZE_ROW;\n vec2 r = vec2(uniformsf[i].x, uniformsf[i].y);\n return r;\n}\nvec2 getScreenSizeInv(){\n const int i = SCREEN_SIZE_ROW;\n vec2 r = vec2(uniformsf[i].z, uniformsf[i].w);\n return r;\n}\nvec2 getAttributeMinMax(){\n const int i = ATTRIBUTE_MINMAX_ROW;\n return vec2(uniformsf[i].x, uniformsf[i].y);\n}\nbool getHiddenLineFlag(){\n return (uniformsint[0].x & 1) == 1;\n}\nbool getShadingFlag(){\n return (uniformsint[0].x & 2) == 2;\n}\nbool getTextureFlag(){\n return (uniformsint[0].x & 4) == 4;\n}\nbool getFlipNormalFlag(){\n return (uniformsint[0].x & 8) == 8;\n}\nvec3 blendLineColorByDistance(vec3 ptdist, vec3 facecolor){\n vec3 fragcolor = facecolor;\n bool hiddenLineFlag = getHiddenLineFlag();\n if (hiddenLineFlag){ \n vec3 linecolor = getLineColor();\n float linewidth = getLineWidth();\n float d = min(ptdist[0], min(ptdist[1], ptdist[2]));\n d -= min(linewidth * 0.5, linewidth-1.0);\n d = max(0.0, d);\n float I = exp(-d * d);\n fragcolor = mix(fragcolor, linecolor, I);\n }\n return fragcolor;\n}\n#endif\n";
    shadersMap["common_uniforms_glsl"]=common_uniforms_glsl;
    static const char* flatshading_vp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\nvoid main(void)\n{\n vec4 pos = vec4(gl_Vertex.xyz, 1.0);\n gl_Position = gl_ModelViewProjectionMatrix * pos; \n \n gl_TexCoord[0] = pos;\n}\n#endif\n";
    shadersMap["flatshading_vp_glsl"]=flatshading_vp_glsl;
    static const char* flatshading_fp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\n#extension GL_ARB_texture_rectangle : enable\n#include \"common_uniforms.glsl\"\n#include \"normal_decompression.glsl\"\n#include \"shading.glsl\"\nuniform sampler2DRect texElmNormal; //Element normal texture;\nnoperspective in vec3 dist;\nvoid main(void)\n{\n vec2 elmAttribMinMax = getAttributeMinMax();\n vec4 frontColor = vec4(getFaceFrontColor(), 1.0);\n vec4 backColor = vec4(getFaceBackColor(), 1.0);\n vec4 vertexPos = vec4(gl_TexCoord[0].xyz, 1.0);\n int pid = gl_PrimitiveID;\n int texwidth = textureSize(texElmNormal).x;\n ivec2 texcoord2 = ivec2(pid % texwidth, pid / texwidth);\n vec2 texel = texelFetch(texElmNormal, texcoord2).xy;\n vec3 norm = decompress_normal(floatBitsToUint(texel.x)); \n \n vec4 facecolor = frontColor;\n if (!gl_FrontFacing) facecolor = backColor;\n vec4 fragcolor = facecolor;\n fragcolor.xyz = shadeVertex(vertexPos, norm, facecolor.xyz);\n \n int s = int(4.0 * texel.y / ( elmAttribMinMax.y - elmAttribMinMax.x));\n if (s <= 0){\n fragcolor.x += 0.35;\n }\n else if (s == 1){\n fragcolor.y += 0.35;\n }\n else if (s == 2){\n fragcolor.z += 0.35;\n }\n else{\n fragcolor.xy += 0.35;\n }\n \n fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);\n gl_FragColor = fragcolor;\n \n}\n#endif\n";
    shadersMap["flatshading_fp_glsl"]=flatshading_fp_glsl;
    static const char* gouraudshading_vp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\n#include \"common_uniforms.glsl\"\n#include \"normal_decompression.glsl\"\n#include \"shading.glsl\"\nvoid main(void)\n{\n vec4 frontColor = vec4(getFaceFrontColor(), 1.0);\n vec4 backColor = vec4(getFaceBackColor(), 1.0);\n vec4 pos = vec4(gl_Vertex.xyz, 1);\n vec3 norm = decompress_normal(floatBitsToUint(gl_Vertex.w));\n vec3 fragFrontColor = shadeVertex(gl_Vertex, norm, frontColor.xyz);\n vec3 fragBackColor = shadeVertex(gl_Vertex, norm, backColor.xyz);\n \n gl_Position = gl_ModelViewProjectionMatrix * pos; \n gl_FrontColor = vec4(fragFrontColor, 1.0);\n gl_FrontSecondaryColor = vec4(fragBackColor, 1.0);\n}\n#endif\n";
    shadersMap["gouraudshading_vp_glsl"]=gouraudshading_vp_glsl;
    static const char* gouraudshading_fp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\n#extension GL_ARB_texture_rectangle : enable\n#include \"common_uniforms.glsl\"\nnoperspective in vec3 dist;\nvoid main(void)\n{\n vec4 fragcolor = gl_Color; \n if (!gl_FrontFacing){\n fragcolor.xyz = gl_SecondaryColor.xyz;\n }\n \n fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);\n gl_FragColor = fragcolor;\n}\n#endif\n";
    shadersMap["gouraudshading_fp_glsl"]=gouraudshading_fp_glsl;
    static const char* hiddenline_gp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 \n#extension GL_EXT_geometry_shader4 : enable\n#include \"common_uniforms.glsl\"\nlayout(triangles) in;\nlayout(triangle_strip, max_vertices=3) out;\n/*\nvarying out vec4 gl_FrontColor;\nvarying out vec4 gl_BackColor;\nvarying out vec4 gl_FrontSecondaryColor;\nvarying out vec4 gl_BackSecondaryColor;\nvarying out vec4 gl_TexCoord[]; // at most gl_MaxTextureCoords\nvarying out float gl_FogFragCoord;\n*/\nnoperspective out vec3 dist;\nout int gl_PrimitiveID; //It needs to be declared before use\nvarying out vec4 gl_TexCoord[2]; //It needs to be declared before use\nvec3 calcScreenSpaceDistances(vec4 vertex[3])\n{\n //get uniform parameters of the screen size\n vec2 screenSize = getScreenSize();\n	vec2 p0 = screenSize * vertex[0].xy / vertex[0].w;\n	vec2 p1 = screenSize * vertex[1].xy / vertex[1].w;\n	vec2 p2 = screenSize * vertex[2].xy / vertex[2].w;\n	vec2 v0 = p2 - p1;\n	vec2 v1 = p2 - p0;\n	vec2 v2 = p1 - p0;\n mat2 m;\n m[0] = v1; m[1] = v2;\n	float area = abs(determinant(m));\n return vec3(area) / vec3(length(v0), length(v1), length(v2));\n}\nvoid emitPerVertexInfo(int id)\n{\n	gl_Position = gl_in[id].gl_Position;\n	gl_FrontColor = gl_in[id].gl_FrontColor;\n	gl_FrontSecondaryColor = gl_in[id].gl_FrontSecondaryColor;	\n	gl_TexCoord[0] = gl_in[id].gl_TexCoord[0];\n	gl_TexCoord[1] = gl_in[id].gl_TexCoord[1];\n	EmitVertex();\n}\nvoid main(void)\n{\n vec4 vertex[3];\n vertex[0] = gl_PositionIn[0];\n vertex[1] = gl_PositionIn[1];\n vertex[2] = gl_PositionIn[2];\n vec3 d = calcScreenSpaceDistances(vertex);\n	gl_PrimitiveID = gl_PrimitiveIDIn;	\n	dist = vec3(d.x, 0, 0);\n emitPerVertexInfo(0);\n	\n	dist = vec3(0, d.y, 0);\n emitPerVertexInfo(1);\n	\n	dist = vec3(0, 0, d.z);\n emitPerVertexInfo(2);\n	EndPrimitive();	\n}\n#endif\n";
    shadersMap["hiddenline_gp_glsl"]=hiddenline_gp_glsl;
    static const char* normal_decompression_glsl="const uint NORMAL_BITS = 12u;\nconst uint NORMAL_MAXI = (1u << NORMAL_BITS) - 1u;\nconst uint NORMAL_MAXI_2 = NORMAL_MAXI >> 1;\n#if !defined(__STDC__) && !defined(__cplusplus)\nvec3 decompress_normal(uint normi)\n{\n uint u = uint(normi & NORMAL_MAXI);\n	uint v = uint((normi >> NORMAL_BITS) & NORMAL_MAXI_2);\n if (u + v >= NORMAL_MAXI){\n u = NORMAL_MAXI - u;\n v = NORMAL_MAXI - v;\n }\n uint w = uint(NORMAL_MAXI - 1u - u - v);\n	vec3 n = vec3(uvec3(u, v, w));\n	uvec3 signs = uvec3(normi>>29, normi>>30, normi>>31) & 1u;\n	return normalize(mix(n, -n, vec3(signs)));\n}\n#endif\n";
    shadersMap["normal_decompression_glsl"]=normal_decompression_glsl;
    static const char* phongshading_vp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\n#include \"common_uniforms.glsl\"\n#include \"normal_decompression.glsl\"\n#include \"shading.glsl\"\nvoid main(void)\n{\n vec4 frontColor = vec4(getFaceFrontColor(), 1.0);\n vec4 backColor = vec4(getFaceBackColor(), 1.0);\n vec4 pos = vec4(gl_Vertex.xyz, 1.0);\n vec3 norm = decompress_normal(floatBitsToUint(gl_Vertex.w));\n vec3 fragFrontColor = shadeVertex(gl_Vertex, norm, frontColor.xyz);\n vec3 fragBackColor = shadeVertex(gl_Vertex, norm, backColor.xyz);\n \n gl_Position = gl_ModelViewProjectionMatrix * pos; \n gl_FrontColor = vec4(fragFrontColor, 1.0);\n gl_FrontSecondaryColor = vec4(fragBackColor, 1.0);\n gl_TexCoord[0] = pos;\n gl_TexCoord[1].xyz = norm;\n}\n#endif\n";
    shadersMap["phongshading_vp_glsl"]=phongshading_vp_glsl;
    static const char* phongshading_fp_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\n#version 330 compatibility \n#extension GL_EXT_gpu_shader4 : enable\n#extension GL_ARB_texture_rectangle : enable\n#include \"common_uniforms.glsl\"\n#include \"shading.glsl\"\nnoperspective in vec3 dist;\nvoid main(void)\n{\n vec4 vertexPos = gl_TexCoord[0];\n vec3 vertexNorm = gl_TexCoord[1].xyz;\n vec4 facecolor = gl_Color;\n if (!gl_FrontFacing){\n facecolor.xyz = gl_SecondaryColor.xyz;\n }\n \n vec4 fragcolor = facecolor;\n fragcolor.xyz = shadeVertex(vertexPos, vertexNorm, facecolor.xyz);\n fragcolor.xyz = blendLineColorByDistance(dist, fragcolor.xyz);\n gl_FragColor = fragcolor;\n \n}\n#endif\n";
    shadersMap["phongshading_fp_glsl"]=phongshading_fp_glsl;
    static const char* shading_glsl="#if !defined(__STDC__) && !defined(__cplusplus)\nvec3 shadeVertex(vec4 vertex, vec3 normal, vec3 facecolor)\n{\n vec3 fragcolor = facecolor * 0.75;\n if (getShadingFlag()){\n vec3 viewdir = vec3(1.0, 1.0, 0.10);\n viewdir = normalize(viewdir);\n if (getFlipNormalFlag()) normal = -normal;\n float diffuse = abs(dot(viewdir, normal)) + 0.20;\n fragcolor = facecolor * diffuse;\n }\n return fragcolor;\n}\n#endif\n";
    shadersMap["shading_glsl"]=shading_glsl;
    return 0;
}//function shaderSourceInit
static int runShaderSourceInit=shaderSourceInit();
}//namespace MeshRender