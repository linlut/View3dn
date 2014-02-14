#pragma once
#ifndef MESH_RENDER_H
#define MESH_RENDER_H

namespace MeshRender{

class MeshRender
{
public:
    MeshRender();
    ~MeshRender();
	static int initRendering();

    //
    void setVertexPointer(const float *vertex, int nv);
    //
    void setVertexPointer(const double *vertexDbl, int nv);
    //
    void setElementPointer(const int *element, int nelm, int vertPerElm);
    //
    void setNormalPointer(const float *normal);
    //
    void setElementNormalPointer(const float *elmNormal);
    //
    void setElementAttributePointer(const float *elmAttribute);
    //
    void setVertexAttributeMinMax(float minval, float maxval);
    //
    void setElementAttributeMinMax(float minval, float maxval);
    //
    void createGPUBuffers();       
	//
	void drawGouraudShaded(const float frontColor[3]);
    //
	void drawFlatShaded(const float frontColor[3]);
    //
	void drawPhongShaded(const float frontColor[3]);
    //
	void drawWireFrame(const float frontColor[3], float lineWidth);
    //
	void drawFlatHiddenLine(const float frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight);
    //
	void drawGouraudHiddenLine(const float frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight);
    //
	void drawPhongHiddenLine(const float frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight);

    inline unsigned int vbo(void)
    {
        return m_vboID;
    }

    inline unsigned int ebo(void)
    {
        return m_eboID; 
    }

protected:
    //disable copy constructor
    MeshRender(const MeshRender& another);

    int m_nv;                   //# of vertices, where each vertex has <x,y,z> components
    const float *m_vertex;      //vertex array pointer, in fact, Vector3f*
    const double *m_vertexDbl;  //vertex array pointer, if double precision is used, Vector3d*
    int m_nelm;                 //# of polygons
    const int *m_element;       //polygon buffer pointer
    int m_vertPerElm;           //# of vertices in each polygon. It should be 3 or 4 only.
    const float *m_normal;      //vertex normal array
    const float *m_elmNormal;   //element normal array, in fact, it is Vector3f*
    const float *m_elmAttribute;//element attribute array, in fact, it is float*

    float m_vertAttribMinMax[2];    //per vertex attribute [Lower, Upper] bound
    float m_elmAttribMinMax[2];     //per element attribute [Lower, Upper] bound

    unsigned int m_vboID;           //GPU vertex bufer object ID
    unsigned int m_eboID;           //GPU element buffer object ID
    unsigned int m_elmNormalTexID;  //element normal texture, which is a compressed 2D texture 
};

}//end namespace MeshRender

#endif