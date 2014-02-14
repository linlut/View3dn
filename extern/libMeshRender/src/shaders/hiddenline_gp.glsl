#if !defined(__STDC__) && !defined(__cplusplus)
#version 330 
#extension GL_EXT_geometry_shader4 : enable

#include "common_uniforms.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

/*
varying out vec4 gl_FrontColor;
varying out vec4 gl_BackColor;
varying out vec4 gl_FrontSecondaryColor;
varying out vec4 gl_BackSecondaryColor;
varying out vec4 gl_TexCoord[]; // at most gl_MaxTextureCoords
varying out float gl_FogFragCoord;
*/
noperspective out vec3 dist;
out int gl_PrimitiveID;             //It needs to be declared before use
varying out vec4 gl_TexCoord[2];    //It needs to be declared before use

vec3 calcScreenSpaceDistances(vec4 vertex[3])
{
    //get uniform parameters of the screen size
    vec2 screenSize = getScreenSize();
	vec2 p0 = screenSize * vertex[0].xy / vertex[0].w;
	vec2 p1 = screenSize * vertex[1].xy / vertex[1].w;
	vec2 p2 = screenSize * vertex[2].xy / vertex[2].w;
	vec2 v0 = p2 - p1;
	vec2 v1 = p2 - p0;
	vec2 v2 = p1 - p0;
    mat2 m;
    m[0] = v1; m[1] = v2;
	float area = abs(determinant(m));
    return vec3(area) / vec3(length(v0), length(v1), length(v2));
}

void emitPerVertexInfo(int id)
{
	gl_Position             = gl_in[id].gl_Position;
	gl_FrontColor           = gl_in[id].gl_FrontColor;
	gl_FrontSecondaryColor  = gl_in[id].gl_FrontSecondaryColor;	
	gl_TexCoord[0]          = gl_in[id].gl_TexCoord[0];
	gl_TexCoord[1]          = gl_in[id].gl_TexCoord[1];
	EmitVertex();
}

void main(void)
{
    vec4 vertex[3];
    vertex[0] = gl_PositionIn[0];
    vertex[1] = gl_PositionIn[1];
    vertex[2] = gl_PositionIn[2];
    vec3 d = calcScreenSpaceDistances(vertex);
	gl_PrimitiveID = gl_PrimitiveIDIn;	

	dist = vec3(d.x, 0, 0);
    emitPerVertexInfo(0);
	
	dist = vec3(0, d.y, 0);
    emitPerVertexInfo(1);
	
	dist = vec3(0, 0, d.z);
    emitPerVertexInfo(2);

	EndPrimitive();	
}

#endif