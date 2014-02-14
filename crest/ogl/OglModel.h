#ifndef _CIA3DFRAMEWORK_OGLMODEL_H
#define _CIA3DFRAMEWORK_OGLMODEL_H

#include <vector>
#include <iostream>
#include <string>
#include <crest/base/vec.h>
#include <crest/base/vectorall.h>
#include <crest/base/Quat.h>
#include <crest/core/RigidTypes.h>
#include <crest/core/VisualModel.h>
#include <crest/core/BaseObjectDescription.h>
#include <crest/core/MappedModel.h>
#include <crest/ogl/texture.h>
#include <crest/io/Mesh.h>
//#include <crest/ogl/OglMaterial.h>
#include <oglmaterial.h>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

namespace cia3d{
namespace ogl{


/// Resizable custom vector class.
/*template<class T>
class ResizableExtVector : public vector<T>
{
public:
	~ResizableExtVector()
	{
		if (this->data!=NULL) delete[] this->data;
	}
	T* getData() { return this->data; }
	const T* getData() const { return this->data; }
	virtual void resize(typename ExtVector<T>::size_type size)
	{
		if (size > this->maxsize)
		{
			T* oldData = this->data;
			this->maxsize = (size > 2*this->maxsize ? size : 2*this->maxsize);
			this->data = new T[this->maxsize];
			for (typename ExtVector<T>::size_type i = 0 ; i < this->cursize ; ++i)
				this->data[i] = oldData[i];
			if (oldData!=NULL) delete[] oldData;
		}
		this->cursize = size;
	}
	void push_back(const T& v)
	{
		int i = this->size();
		resize(i+1);
		(*this)[i] = v;
	}
};
*/
#define ResizableExtVector std::vector

class OglModel : public cia3d::core::VisualModel
				 //public cia3d::zcore::MappedModel<ExtVectorTypes< Vec<3,GLfloat>, Vec<3,GLfloat> > >
{
private:
	typedef Vector2f TexCoord;
	typedef Vector3d Coord;
	typedef Vector3i Triangle;
	typedef Vector4i Quad;

	typedef vector<Coord> VecCoord;
	typedef vector<Coord> VecDeriv;

	ResizableExtVector<Coord>* inputVertices;

	bool modified; ///< True if input vertices modified since last rendering
	bool useTopology; ///< True if list of facets should be taken from the attached topology
	bool useNormals; ///< True if normals should be read from file
	bool castShadow; ///< True if object cast shadows

	ResizableExtVector<Coord> vertices;
	ResizableExtVector<Coord> vnormals;
	ResizableExtVector<TexCoord> vtexcoords;

	ResizableExtVector<Triangle> triangles;
	ResizableExtVector<Quad> quads;

	/// If vertices have multiple normals/texcoords, then we need to separate them
	/// This vector store which input position is used for each vertice
	/// If it is empty then each vertex correspond to one position
	ResizableExtVector<int> vertPosIdx;

	/// Similarly this vector store which input normal is used for each vertice
	/// If it is empty then each vertex correspond to one normal
	ResizableExtVector<int> vertNormIdx;

	ogl::Material material;

	ogl::Texture *tex;

	void internalDraw();

public:

	OglModel();

	~OglModel();

    void parse(ZBaseObjectDescription* arg);

	bool isTransparent();

 	void draw();
	void drawTransparent();
	void drawShadow();

	bool load(const std::string& filename, const std::string& loader, const std::string& textureName);

	void applyTranslation(double dx, double dy, double dz);
	void applyRotation(const Quat* q);
	void applyScale(double s);
	void applyUVScale(double su, double sv);
	void computeNormals();

	void flipFaces();
	
	void setColor(float r, float g, float b, float a);
	void setColor(std::string color);

	void setUseNormals(bool val) { useNormals = val;  }
	bool getUseNormals() const   { return useNormals; }

	void setCastShadow(bool val) { castShadow = val;  }
	bool getCastShadow() const   { return castShadow; }

	void update();

	void init();

	void initTextures();

	bool addBBox(double* minBBox, double* maxBBox);
	
	const VecCoord* getX()  const; // { return &x;   }
	const VecDeriv* getV()  const { return NULL; }
	/*
	const VecDeriv* getF()  const { return NULL; }
	const VecDeriv* getDx() const { return NULL; }
	*/
	
	VecCoord* getX(); //  { return &x;   }
	VecDeriv* getV()  { return NULL; }
	/*
	VecDeriv* getF()  { return NULL; }
	VecDeriv* getDx() { return NULL; }
	*/
	
	/// Append this mesh to an OBJ format stream.
	/// The number of vertices position, normal, and texture coordinates already written is given as parameters
	/// This method should update them
	virtual void exportOBJ(std::string name, std::ostream* out, std::ostream* mtl, int& vindex, int& nindex, int& tindex);
};

typedef Vec<3,GLfloat> GLVec3f;
//typedef ExtVectorTypes<GLVec3f,GLVec3f> GLExtVec3fTypes;


} // namespace ogl
} // namespace cia3d

#endif
