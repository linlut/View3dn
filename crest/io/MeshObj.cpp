#include <stdlib.h>
#include <crest/core/Config.h>
#include <crest/io/MeshObj.h>
#include <crest/system/FileRepository.h>

namespace cia3d
{
namespace io
{

//using namespace sofa::defaulttype;

CIA3D_DECL_CLASS(MeshOBJ)

cia3d::core::Creator<Mesh::Factory, MeshOBJ> MeshOBJClass("obj");

void MeshOBJ::init (std::string filename)
{
	FILE *f = fopen(filename.c_str(), "r");
	if (f){
		readOBJ(f);
		fclose(f);
	}
	else
		std::cerr << "File " << filename << " not found " << std::endl;
}

// -------------------------------------------------------------------------
// _glmFirstPass: first pass at a Wavefront OBJ file that gets all the
//  			  statistics of the model (such as #vertices, #normals, etc)
//
//  	model - properly initialized GLMmodel structure
//  	file  - (fopen'd) file descriptor 
// --------------------------------------------------------------------------
void MeshOBJ::readOBJ (FILE* file)
{
	vector< vector<int> > vertNormTexIndices;
	vector<int>vIndices, nIndices, tIndices;
	int vtn[3];
	char buf[512], matName[1024];
	Vector3d result(0,0,0);
	Vector3f texCoord(0,0,0), normal(0,0,0);
	const char *token;

	std::string face, tmp;

	while (fscanf(file, "%s", buf) != EOF)
	{
		switch (buf[0])
		{
			case '#':
				/* comment */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			case 'v':
				/* v, vn, vt */
				switch (buf[1])
				{
					case '\0':
						/* vertex */
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						sscanf(buf, "%lf %lf %lf", &result[0], &result[1], &result[2]);
						vertices.push_back(result);
						break;
					case 'n':
						/* normal */
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						sscanf(buf, "%f %f %f", &normal[0], &normal[1], &normal[2]);
						normals.push_back(normal);
						break;
					case 't':
						/* texcoord */
						/* eat up rest of line */
						fgets(buf, sizeof(buf), file);
						sscanf (buf, "%f %f", &texCoord[0], &texCoord[1]);
						texCoords.push_back(texCoord);
						break;
					default:
						printf("readObj : Unknown token \"%s\".\n", buf);
						exit(1);
						break;
				}
				break;
			case 'm':
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s %s", buf, buf);
				readMTL(buf);
				break;
			case 'u':
				{
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					sscanf(buf, "%s", matName);
					vector<ogl::Material>::iterator it = materials.begin();
					vector<ogl::Material>::iterator itEnd = materials.end();
					for (; it != itEnd; it++)
					{
						if (strcmp ((*it).name.c_str(), matName) == 0)
						{
							material = (*it);
							material.activated = true;
						}
					}
				}
				break;
			case 'g':
				/* group */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				sscanf(buf, "%s", buf);
				break;
			case 'l': // for now we consider a line as a 2-vertices face
			case 'f':
				// face 
				fgets(buf, sizeof(buf), file);
				token = strtok(buf, " ");
				
				vIndices.clear();
				nIndices.clear();
				tIndices.clear();
				vertNormTexIndices.clear();
				
				while(token!=NULL && token[0]>='0' && token[0]<='9')
				{
					face = token;
					for (int j = 0; j < 3; j++)
					{
						vtn[j] = 0;
						std::string::size_type pos = face.find('/');
						tmp = face.substr(0, pos);
						if (tmp != "")
							vtn[j] = atoi(tmp.c_str()) - 1; // -1 because the numerotation begins at 1 and a vector begins at 0
						if (pos == std::string::npos)
							face = "";
						else
							face = face.substr(pos + 1);
					}
					vIndices.push_back(vtn[0]);
					nIndices.push_back(vtn[1]);
					tIndices.push_back(vtn[2]);
					token = strtok(NULL, " ");
				}
				vertNormTexIndices.push_back (vIndices);
				vertNormTexIndices.push_back (nIndices);
				vertNormTexIndices.push_back (tIndices);
				facets.push_back(vertNormTexIndices);
				break;

			default:
				// eat up rest of line 
				fgets(buf, sizeof(buf), file);
				break;
		}
	}

	// announce the model statistics 
	std::cout << " Vertices: " << vertices.size() << std::endl;
	std::cout << " Normals: " << normals.size() << std::endl;
	std::cout << " Texcoords: " << texCoords.size() << std::endl;
	std::cout << " Triangles: " << facets.size() << std::endl;
	if (vertices.size()>0){
		// compute bbox
		Vector3d minBB = vertices[0];
		Vector3d maxBB = vertices[0];
		for (unsigned int i=1; i<vertices.size(); i++)
		{
			Vector3d p = vertices[i];
			for (int c=0;c<3;c++)
			{
				if (minBB[c] > p[c])
					minBB[c] = p[c];
				if (maxBB[c] < p[c])
					maxBB[c] = p[c];
			}
		}
		
		std::cout << "BBox: <"<<minBB[0]<<','<<minBB[1]<<','<<minBB[2] 
		          <<">-<"
				  <<maxBB[0]<<','<<maxBB[1]<<','<<maxBB[2]<<">\n";
	}

}

// -----------------------------------------------------
// readMTL: read a wavefront material library file
//
//    model - properly initialized GLMmodel structure
//    name  - name of the material library
// -----------------------------------------------------
void MeshOBJ::readMTL(char* filename)
{
	char buf[512];
	FILE *file = fopen(filename, "r");
	ogl::Material *mat = NULL;
	if (!file){
		std::cerr << "readMTL() failed: can't open material file " << filename << std::endl;
	}
	else{
		/* now, read in the data */
		while (fscanf(file, "%s", buf) != EOF){
			switch (buf[0]){
				case '#':
					/* comment */
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					break;
				case 'n':
					/* newmtl */
					if (mat != NULL) {
						materials.push_back(*mat);
						delete mat;
						mat = NULL;
					}
					mat = new ogl::Material();
					fgets(buf, sizeof(buf), file);
					sscanf(buf, "%s %s", buf, buf);
					mat->name = buf;
					break;
				case 'N':
					switch (buf[1]) {
						case 'i':
						{
							double optical_density;
							fscanf(file, "%lf", &optical_density);
							break;
						}
						case 's':
							fscanf(file, "%lf", &mat->shininess);
							mat->useShininess = true;
							break;
						default:
							/* eat up rest of line */
							fgets(buf, sizeof(buf), file);
							break;
					}
					break;
				case 'K':
					switch (buf[1]) {
						case 'd':
							fscanf(file, "%lf %lf %lf", &mat->diffuse[0], &mat->diffuse[1], &mat->diffuse[2]);
							mat->useDiffuse = true;
							break;
						case 's':
							fscanf(file, "%lf %lf %lf", &mat->specular[0], &mat->specular[1], &mat->specular[2]);
							mat->useSpecular = true;
							break;
						case 'a':
							fscanf(file, "%lf %lf %lf", &mat->ambient[0], &mat->ambient[1], &mat->ambient[2]);
							mat->useAmbient = true;
							break;
						default:
							/* eat up rest of line */
							fgets(buf, sizeof(buf), file);
							break;
					}
					break;
				case 'd':
				case 'T':
					// transparency value
					fscanf(file, "%lf", &mat->diffuse[3]);
					break;
				default:
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					break;
			}
			
		}
		fclose(file);
	}

	if (mat != NULL) {
		materials.push_back(*mat);
		delete mat;
		mat = NULL;
	}
}

} // namespace io
} // namespace cia3d

