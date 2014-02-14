#ifndef __ZZIO_MESH_H
#define __ZZIO_MESH_H

#include <vector>
#include <string>
#include <iostream>
#include <crest/base/vectorall.h>
#include <crest/ogl/OglMaterial.h>
#include <crest/core/Factory.h>
#include <crest/core/Factory.inl>

using namespace std;

namespace cia3d{
namespace io{

class Mesh
{
public:
	
protected:
	//Mesh data
	vector<Vector3d> vertices;
	vector<Vector3f> texCoords; //Suppose texCoords is order 2 (2 texCoords for a vertex)
	vector<Vector3f> normals;
	vector< vector < vector <int> > > facets;

	//Material info.
	ogl::Material material;
	
	//Texture info.
	std::string textureName;

public:
	
	vector<Vector3d> & getVertices() { return vertices; };
	vector<Vector3f> & getTexCoords() { return texCoords; };
	vector<Vector3f> & getNormals() { return normals;};
	vector< vector < vector <int> > > & getFacets(){ return facets; };

	ogl::Material& getMaterial() {return material;};
	
	std::string& getTextureName(){ return textureName; };
	
	typedef cia3d::core::Factory< std::string, Mesh, std::string > Factory;
	static Mesh* Create(std::string filename)
	{
		std::string loader="default";
		std::string::size_type p = filename.rfind('.');
		if (p!=std::string::npos)
			loader = std::string(filename, p+1);
		return Factory::CreateObject(loader, filename);
	}

};

} // namespace io
} // namespace cia3d

#endif
