#include "vector4f.h"
#include "vector3f.h"
#include "vector3d.h"
#include "vector2f.h"
#include "MeshRenderingContext.h"
#include "NormalCompresser.h"
#include "MeshRender.h"

#ifndef CHECK_GL_ERRORS
#define CHECK_GL_ERRORS  CheckGLError
#endif

#ifndef nullptr 
#define nullptr 0
#endif

typedef unsigned int uint;

inline unsigned int upper_power_of_two(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void calcElementTextureSize(unsigned int nelm, int dims[2])
{
    const int dimx = int(sqrt(double(nelm)) + 0.5);
    int dimx_pow2 = upper_power_of_two(dimx);
    if (dimx_pow2 > 512){
        GLint maxTextureSize[4];
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);
        if (dimx_pow2 > maxTextureSize[0]){
            dimx_pow2 = maxTextureSize[0];
            std::cerr << "Error: texture size larger than GL_MAX_TEXTURE_SIZE=" 
                      << maxTextureSize[0] << "!\n";
        }
    }
    
    dims[0] = dimx_pow2;
    dims[1] = nelm / dimx_pow2;
    if (nelm % dimx_pow2 != 0) dims[1]++;
    if ((dims[1] & 0x1) == 1) dims[1]++;
    if (dims[0] < 4) dims[0] = 4;
    if (dims[1] < 2) dims[1] = 2;
}


static GLuint genElementNormalTextureWithNormalCompression(unsigned int nelm, const float *normal, const float *attribute)
{
    if (0 == nelm || (nullptr == normal && nullptr == attribute)) return 0;

    int texdims[2];
    calcElementTextureSize(nelm, texdims);
    GLuint mTexID = 0;
	glGenTextures(1, &mTexID);
    const GLuint target = GL_TEXTURE_RECTANGLE;
    glBindTexture(target, mTexID);
    glTexImage2D(target, 0, GL_RG32F, texdims[0], texdims[1], 0, GL_RG, GL_FLOAT, NULL);
    CHECK_GL_ERRORS;

    const uint BUFFER_SIZE = 16384;
    Vector2f elmNormals[BUFFER_SIZE];
    const uint rownum = BUFFER_SIZE / texdims[0];
    if (rownum == 0){
        std::cerr << "Error: normal buffer is too small, in file " << __FILE__ << __LINE__ << "!\n";
        glDeleteTextures(1, &mTexID);
        return 0;
    }
    uint loopnum = texdims[1]/rownum; 
    if (texdims[1]%rownum != 0) loopnum++;
    const uint runlength = rownum * (uint)texdims[0];
    const Vector3f *srcNormal = (const Vector3f*)normal;
    for (uint j=0; j<loopnum; j++){
        {
            const uint startPos = j * runlength;
            const uint len = _MIN_(runlength, nelm - startPos);
            for (uint i=0; i<len; i++){
                const uint idx = i + startPos;
                Vector2f &dst = elmNormals[i];
                dst = Vector2f(0.0f, 0.0f);
                if (nullptr != srcNormal){
		            const uint32_t norm_uint = MeshRender::compress_normal(&srcNormal[idx].x);
		            dst.x = MeshRender::reinterprete_uint32_as_float(norm_uint);
                }
                if (nullptr != attribute){
                    dst.y = attribute[idx];
                }
                dst.y = float(idx);    
            }
            for (uint i=len; i<runlength; i++) elmNormals[i] = Vector2f(0, 0);
        }
        {
            const uint xoffset = 0;
            const uint yoffset = j*rownum;
            const uint imgwidth = texdims[0];
            const uint imgheight = _MIN_(rownum, texdims[1] - j*rownum);
            glTexSubImage2D(target, 0, xoffset, yoffset, imgwidth, imgheight, GL_RG, GL_FLOAT, elmNormals);
            CHECK_GL_ERRORS;
        }
    }

	glBindTexture(target, 0);   
	return mTexID;
}


static GLuint createVBO(size_t vertexSize, const float* vertex, const float *normal, const float* color)
{
    const GLenum target = GL_ARRAY_BUFFER;
    const GLenum usage = GL_STREAM_DRAW_ARB;
    GLuint id = 0;                               // 0 is reserved, 
    glGenBuffers(1, &id);                        // returns non-zero id if success
    glBindBuffer(target, id);                    // activate vbo id to use
    if (1){
        size_t totalSize = vertexSize * 3;
        if (color == NULL) totalSize = vertexSize * 2;
        glBufferData(target, totalSize, 0, usage);
        size_t offset = 0;
        glBufferSubData(target, offset, vertexSize, vertex);
        offset += vertexSize;
        glBufferSubData(target, offset, vertexSize, normal);
        offset += vertexSize;
        if (color != NULL){
            glBufferSubData(target, offset, vertexSize, color);
        }
        CHECK_GL_ERRORS;
    }
	glBindBuffer(target, 0);
    return id;
}


static GLuint createVBOWithNormalCompression(
    size_t nv, const float* vertex, const double *vertexDbl, const float *normal)
{
	if (0 == nv || (nullptr == vertex && nullptr == vertexDbl)) return 0;

	const GLenum target = GL_ARRAY_BUFFER;
	const GLenum usage = GL_STATIC_DRAW;
	GLuint id = 0;                               // 0 is reserved id
	glGenBuffers(1, &id);                        // returns non-zero id if success
	glBindBuffer(target, id);                    // activate vbo id to use

	const size_t totalSize = sizeof(float) * nv * 4;
	glBufferData(target, totalSize, 0, usage);

	//create a buffer to store <x,y,z,compressed-norm>
	//where the compressed-norm is a floating point number
	const int BUFFER_LENGTH = 8192;
	const int BUFFER_MASK = BUFFER_LENGTH - 1;
	Vector4f vbuffer[BUFFER_LENGTH];
	const Vector3f znorm(0, 0, 1.0f);
	const int normal_step = int(normal != nullptr);
	const Vector3f *srcVertex = (const Vector3f *)vertex;
	const Vector3d *srcVertexDbl = (const Vector3d *)vertexDbl;
	const Vector3f *srcNormal = (const Vector3f *)normal;
	if (nullptr == srcNormal) srcNormal = &znorm;
	GLintptr offset = 0;
	for (unsigned int i = 0; i < nv; ++i, srcNormal += normal_step){
        const unsigned int j = i & BUFFER_MASK;
		Vector4f *v = &vbuffer[j];
		const uint32_t norm_uint = MeshRender::compress_normal(&srcNormal->x);
		v->w = MeshRender::reinterprete_uint32_as_float(norm_uint);
        if (nullptr != srcVertex){
		    *((Vector3f*)v) = srcVertex[i];
        }
        else{
            const Vector3d & vd = srcVertexDbl[i];
            *((Vector3f*)v) = Vector3f(float(vd.x), float(vd.y), float(vd.z));
        }
		if (BUFFER_LENGTH - 1 == j || nv - 1 == i){
			GLsizeiptr currentSize = (i + 1) *sizeof(Vector4f);
			glBufferSubData(target, offset, currentSize - offset, vbuffer);
			offset = currentSize;
		}
	}

	//upload to GPU
	CHECK_GL_ERRORS;
	glBindBuffer(target, 0);

	return id;      // return VBO id
}


static GLuint createEBO(const void* data, unsigned int dataSizeInByte)
{
    if (nullptr == data || 0 == dataSizeInByte) return 0;

	const GLsizeiptr dataSize = dataSizeInByte;
    const GLenum target = GL_ELEMENT_ARRAY_BUFFER;
    const GLenum usage = GL_STATIC_DRAW;

    GLuint id = 0;                               // 0 is reserved
    glGenBuffers(1, &id);                        // return non-zero id if success
    glBindBuffer(target, id);                    // activate vbo id to use
    glBufferData(target, dataSize, data, usage); // upload data to video card
    CHECK_GL_ERRORS;

    // check data size in VBO is same as input array, if not return 0 and delete VBO
    int bufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
    if(dataSize != (unsigned int)bufferSize)
    {
        glDeleteBuffers(1, &id);
        id = 0;
        std::cerr << "[createVBO()] Data size is mismatch with input array\n";
    }

	glBindBuffer(target, 0);
    return id;      // return VBO id
}

static void DrawModel(GLuint vboId, GLuint eboId, int nv, int nelm, int vert_per_elm)
{
    const size_t vertexSize = nv * 4 * sizeof(GLfloat);
	const int offset = vertexSize;
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(4, GL_FLOAT, 0, NULL);

    const GLsizei indexSize = nelm * vert_per_elm;
    GLenum mode = GL_TRIANGLES;
    if (vert_per_elm == 4) mode = GL_QUADS;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	glDrawElements(mode, indexSize, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
}


//=================================================================
namespace MeshRender{

MeshRender::MeshRender():
    m_nv(0)
    ,m_vertex(nullptr)
    ,m_vertexDbl(nullptr)
    ,m_nelm(0)
    ,m_element(nullptr)
    ,m_vertPerElm(0)
    ,m_normal(nullptr)
    ,m_elmNormal(nullptr)
    ,m_elmAttribute(nullptr)
    ,m_vboID(0)
    ,m_eboID(0)
    ,m_elmNormalTexID(0)
{
    m_vertAttribMinMax[0] = 0.0f; 
    m_vertAttribMinMax[1] = 1.0f; 
    m_elmAttribMinMax[0] = 0.0f;
    m_elmAttribMinMax[1] = 1.0f;
}

MeshRender::~MeshRender()
{
    GLuint buffers[128];
    if (1){
        int n = 0;
        if (m_vboID != 0) buffers[n++] = m_vboID;
        if (m_eboID != 0) buffers[n++] = m_eboID;
        glDeleteBuffers(n, buffers);
    }
    if (1){
        int n = 0;
        if (m_elmNormalTexID != 0) buffers[n++] = m_elmNormalTexID;
        glDeleteTextures(n, buffers);
    }
}

int MeshRender::initRendering()
{
	return 1;
}

void MeshRender::setVertexPointer(const float *vertex, int nv)
{
    m_vertex = vertex;
    m_nv = nv;
}

void MeshRender::setVertexPointer(const double *vertexDbl, int nv)
{
    m_vertexDbl = vertexDbl;
    m_nv = nv;
}

void MeshRender::setElementPointer(const int *element, int nelm, int vertPerElm)
{
    m_element = element;
    m_nelm = nelm;
    m_vertPerElm = vertPerElm;
}

void MeshRender::setNormalPointer(const float *normal)
{
    m_normal = normal;
}

void MeshRender::setElementNormalPointer(const float *elmNormal)
{
	m_elmNormal = elmNormal;
}

void MeshRender::setElementAttributePointer(const float *elmAttribute)
{
    m_elmAttribute = elmAttribute;
}

void MeshRender::setVertexAttributeMinMax(float minval, float maxval)
{
    m_vertAttribMinMax[0] = minval;
    m_vertAttribMinMax[1] = maxval;
}

void MeshRender::setElementAttributeMinMax(float minval, float maxval)
{
    m_elmAttribMinMax[0] = minval;
    m_elmAttribMinMax[1] = maxval;
}

void MeshRender::createGPUBuffers()
{
	const unsigned int dataSizeInByte = sizeof(int) * m_nelm * m_vertPerElm;
    if (0 != m_eboID){
        glDeleteBuffers(1, &m_eboID);
    }
    if (0 != m_vboID){
        glDeleteBuffers(1, &m_vboID);
    }
	m_eboID = createEBO(m_element, dataSizeInByte);
	m_vboID = createVBOWithNormalCompression(m_nv, m_vertex, m_vertexDbl, m_normal);

	if (0 != m_elmNormalTexID){ 
		glDeleteTextures(1, &m_elmNormalTexID);
	}
    m_elmNormalTexID = genElementNormalTextureWithNormalCompression(m_nelm, m_elmNormal, m_elmAttribute);
}

void MeshRender::drawGouraudShaded(const float _frontColor[3])
{
    extern GouraudShadingRC gouraudShadingRc;
    GouraudShadingRC &rc = gouraudShadingRc;
    rc.bind(0, 0, 0);
    if (1){
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setHiddenLineFlag(false);
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}

void MeshRender::drawFlatShaded(const float _frontColor[3])
{
    extern FlatShadingRC flatShadingRc;
    FlatShadingRC &rc = flatShadingRc;
    rc.bind(0, 0, 0);
    if (1){
	    rc.setElementNormalTexture(m_elmNormalTexID);
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - 0.5f*frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setAttributeMinMax(0, (float)m_nelm);
        rc.setHiddenLineFlag(false);
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}

void MeshRender::drawPhongShaded(const float _frontColor[3])
{
    extern PhongShadingRC phongShadingRc;
    PhongShadingRC &rc = phongShadingRc;
    rc.bind(0, 0, 0);
    if (1){
	    //rc.setElementNormalTexture(m_elmNormalTexID);
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - 0.5f*frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setAttributeMinMax(0, (float)m_nelm);
        rc.setHiddenLineFlag(false);
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}

void MeshRender::drawWireFrame(const float frontColor[3], float lineWidth)
{
    glLineWidth(lineWidth);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    this->drawFlatShaded(frontColor);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void MeshRender::drawFlatHiddenLine(const float _frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight)
{
    /*
    //classic two-pass rendering
    const float lineColor[3] = {0,0,0};
    this->drawWireFrame(lineColor, lineWidth);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
    this->drawFlatShaded(frontColor);
	glDisable(GL_POLYGON_OFFSET_FILL);
    */

    extern FlatHiddenLineRC fhiddenLineRC;
    FlatHiddenLineRC &rc = fhiddenLineRC;
    rc.bind(0, 0, 0);
    if (1){
	    rc.setElementNormalTexture(m_elmNormalTexID);
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setAttributeMinMax(0, (float)m_nelm);
        rc.setScreenSize(screenWidth, screenHeight);
        rc.setHiddenLineFlag(true);
        rc.setShadingFlag(true);
        rc.setLineWidth(lineWidth);
        rc.setLineColor(*((vec3*)lineColor));
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}


void MeshRender::drawGouraudHiddenLine(const float _frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight)
{
    extern GouraudHiddenLineRC ghiddenLineRC;
    GouraudHiddenLineRC &rc = ghiddenLineRC;
    rc.bind(0, 0, 0);
    if (1){
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setScreenSize(screenWidth, screenHeight);
        rc.setHiddenLineFlag(true);
        rc.setLineWidth(lineWidth);
        rc.setLineColor(*((vec3*)lineColor));
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}


void MeshRender::drawPhongHiddenLine(const float _frontColor[3], const float lineColor[3], float lineWidth, int screenWidth, int screenHeight)
{
    extern PhongHiddenLineRC phiddenLineRC;
    PhongHiddenLineRC &rc = phiddenLineRC;
    rc.bind(0, 0, 0);
    if (1){
        const vec3 frontColor(_frontColor[0], _frontColor[1], _frontColor[2]);
        const vec3 backColor = vec3(1.0f) - frontColor;
        rc.setFaceFrontColor(frontColor);
        rc.setFaceBackColor(backColor);
        rc.setScreenSize(screenWidth, screenHeight);
        rc.setHiddenLineFlag(true);
        rc.setLineWidth(lineWidth);
        rc.setLineColor(*((vec3*)lineColor));
        rc.uploadAllUniformsToGPU();
        DrawModel(m_vboID, m_eboID, m_nv, m_nelm, m_vertPerElm);
    }
    rc.unbind();
}

}//end namespace MeshRender