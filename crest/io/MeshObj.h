//FILE: meshobj.h

#ifndef _CIA3D_ZIO_MESHOBJ_H
#define _CIA3D_ZIO_MESHOBJ_H

#include <crest/io/mesh.h>

namespace cia3d
{
namespace io
{

class MeshOBJ : public Mesh
{
private:

	std::vector<ogl::Material> materials;

	void readOBJ (FILE *file);

	void readMTL (char *filename);

public:
	
	//initialize the mesh structure by reading the OBJ file
	void init (std::string filename);

	//constructor
	MeshOBJ(const std::string& filename)
	{
		init (filename);
	}
};

} // namespace io
} // namespace sofa

#endif
