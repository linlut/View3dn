//FILE: objrw.cpp
//Read/write the wavefront obj file format
#include <string>
#include <sysconf.h>
#include <objrw.h>

using namespace std;


static void _findDirPath(const char *fname, char dirpath[], const int nlen=0)
{
	int i;
	const int slen = strlen(fname);
	for (i=slen-1; i>=0; i--){
		if (fname[i]=='/' || fname[i]=='\\')
			break;
	}
	if (nlen>0 && i>=nlen) assert(0); //not enough buffer size
	for (int j=0; j<=i; j++)
		dirpath[j] = fname[j];
	dirpath[i+1]=0;
}


static OglTexture* _loadTexture(const char *texturename, const char *dirpath, const int textype, const float scale)
{
	OglTexture *ptex = OglTextureFactory::GetEntry(texturename);
	if (ptex == NULL){
		ptex = new OglTexture(texturename, textype, scale);
		if (ptex){
			if (ptex->loadImage())
				OglTextureFactory::AddEntry(texturename, ptex);
			else{
				delete ptex;
				char altname[1024]="";
				strcat(altname, dirpath);
				strcat(altname, texturename);
				ptex = new OglTexture(altname, textype, scale);
				assert(ptex!=NULL);
				if (ptex->loadImage())
					OglTextureFactory::AddEntry(texturename, ptex);
				else{
					delete ptex;
					ptex = NULL;
				}
			}
		}
	}

	return ptex;
}

//Read the material file of obj format
static bool readObjMTL(const char* filename, const char *dirpath, vector<OglMaterial>& materials)
{
	double _x, _y, _z;
	char buf[1024]="";
	OglMaterial *mat = NULL;
	materials.clear();

	FILE *file = fopen(filename, _RA_);
	if (file==NULL){
		strcat(buf, dirpath);
		strcat(buf, filename);
		file = fopen(buf, _RA_);
		if (file==NULL){
			fprintf(stderr, "readMTL() failed: can't open material file %s\n", filename);
			return false;
		}
	}

	//----- now, read in the data -------------------
	while (fscanf(file, "%s", buf) != EOF){
		switch (buf[0]){
			case '#':
				//----------- eat up rest of line -------------
				fgets(buf, sizeof(buf), file);
				break;
			case 'b':	//Read the color map
				if (strcmp(buf, "bump")==0){
					char option[512];
					float scale;
					fscanf(file, "%s %s %d", buf, option, &scale );
					OglTexture* ptex = _loadTexture(buf, dirpath, 1, scale);
					mat->bumptex = ptex;
				}
				else{
					fgets(buf, sizeof(buf), file);
				}
				break;
			case 'm':	
				//-----------Read the color map---------------
				if (strcmp(buf, "map_Kd")==0){
					fscanf(file, "%s", buf);
					OglTexture* ptex = _loadTexture(buf, dirpath, 0, 1);
					mat->colortex = ptex;
				}
				else{
					fgets(buf, sizeof(buf), file);
				}
				break;
			case 'n':
				// ---------------newmtl ------------
				if (mat != NULL) {
					materials.push_back(*mat);
					delete mat;
					mat = NULL;
				}
				mat = new OglMaterial();
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
						fscanf(file, "%lf", &_x);
						mat->shininess = _x;
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
						fscanf(file, "%lf %lf %lf", &_x, &_y, &_z);
						mat->diffuse[0] = _x, 
						mat->diffuse[1] = _y, 
						mat->diffuse[2] = _z;
						mat->useDiffuse = true;
						break;
					case 's':
						fscanf(file, "%lf %lf %lf", &_x, &_y, &_z);
						mat->specular[0] = _x, 
						mat->specular[1] = _y, 
						mat->specular[2] = _z;
						mat->useSpecular = true;
						break;
					case 'a':
						fscanf(file, "%lf %lf %lf", &_x, &_y, &_z);
						mat->ambient[0] = _x, 
						mat->ambient[1] = _y,
						mat->ambient[2] = _z;
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
				fscanf(file, "%lf %lf %lf", &_x, &_y, &_z);
				mat->diffuse[3] = _z;
				break;
			default:
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
		}			
	}

	//close everything
	fclose(file);
	if (mat != NULL){
		materials.push_back(*mat);
		delete mat;
		mat = NULL;
	}
	return true;
}


static void exportOBJMtl(std::ostream* mtl, OglMaterial *materials, const int nmat)
{
	int count = 0;
	for (int i=0; i<nmat; i++){
		const OglMaterial & material = materials[i];
		std::string name = material.name;
		if (name.empty()){
			char buf[256];
			sprintf(buf, "mat%d", ++count);
			materials[i].name = buf;
			name = buf;
		}
		*mtl << "newmtl "<<name<<"\n";
		*mtl << "illum 4\n";
		if (material.useAmbient)
			*mtl << "Ka "<<material.ambient[0]<<' '<<material.ambient[1]<<' '<<material.ambient[2]<<"\n";
		if (material.useDiffuse)
			*mtl << "Kd "<<material.diffuse[0]<<' '<<material.diffuse[1]<<' '<<material.diffuse[2]<<"\n";
		if (material.useSpecular)
			*mtl << "Ks "<<material.specular[0]<<' '<<material.specular[1]<<' '<<material.specular[2]<<"\n";
		*mtl << "Tf 1.00 1.00 1.00\n";
		*mtl << "Ni 1.00\n";
		if (material.useShininess)
			*mtl << "Ns "<<material.shininess<<"\n";
		if (material.useDiffuse && material.diffuse[3]<1.0)
			*mtl << "Tf "<<material.diffuse[3]<<' '<<material.diffuse[3]<<' '<<material.diffuse[3]<<"\n";

		//Need to address the texture and bump map issues;
	}
}


//Limit: the max material buffer length to 32 (2^5)
//Limit: the max polygon numbers are 2^27
static bool 
readOBJMesh(FILE* file, 
	const char *dirpath,
	vector<OglMaterial> &materials,		//Material array
	vector<Vector3d> &vertices,			//Vertex array
	vector<Vector3f> &normals,			//Vertex normal array
	vector<Vector3f> &texCoords,		//Suppose texCoords is order 2 (tx, ty)
	vector< vector < vector <int> > > &facets, 	//Facets array
	unsigned int *pFaceMatIndex,		//The material index buffer, it indicates which polygons are using which mats
	int &nFaceMatIndex)					//The length of the array
{
	vector< vector<int> > vertNormTexIndices;
	vector<int> vIndices, nIndices, tIndices;
	int vtn[3];
	char buf[512], matName[1024];
	Vector3d result(0,0,0);
	const char *token;
	std::string face, tmp;

	//clear the buffers
	nFaceMatIndex = 0;
	materials.clear();
	vertices.clear();
	texCoords.clear();
	facets.clear();

	while (fscanf(file, "%s", buf) != EOF){
		switch (buf[0]){
			case '#':
				/* comment */
				/* eat up rest of line */
				fgets(buf, sizeof(buf), file);
				break;
			case 'v':
				/* v, vn, vt */
				switch (buf[1]){
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
						{
						Vector3f normal;
						fgets(buf, sizeof(buf), file);
						sscanf(buf, "%f %f %f", &normal.x, &normal.y, &normal.z);
						normals.push_back(normal);
						}
						break;
					case 't':
						/* texcoord */
						/* eat up rest of line */
						{
						fgets(buf, sizeof(buf), file);
						Vector3f texCoord(0,0,0); 
						sscanf(buf, "%f %f", &texCoord[0], &texCoord[1]);
						texCoords.push_back(texCoord);
						}
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
				readObjMTL(buf, dirpath, materials);
				break;
			case 'u':
				{
					/* eat up rest of line */
					fgets(buf, sizeof(buf), file);
					sscanf(buf, "%s", matName);
					for (unsigned int i=0; i<materials.size(); i++){
						OglMaterial &material = materials[i];
						if (strcmp(material.name.c_str(), matName) == 0){
							material.activated = true;
							unsigned int facepos = facets.size();
							pFaceMatIndex[nFaceMatIndex] = encodeMaterialIndex(facepos, i);
							nFaceMatIndex ++ ;
							break;
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
				
				while(token!=NULL && token[0]>='0' && token[0]<='9'){
					face = token;
					for (int j = 0; j < 3; j++){
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
	std::cout << " Texcoords: " << texCoords.size() << std::endl;
	std::cout << " Triangles: " << facets.size() << std::endl;
	return true;
}


bool readOBJFile(const char *fname, 
	vector<OglMaterial> &materials, 
	vector<Vector3d> &vertices,
	vector<Vector3f> &normals,
	vector<Vector3f> &texCoords, 
	vector< vector < vector <int> > > &facets, 
	unsigned int *pFaceMatIndex,	//The material index buffer, it indicates which polygons are using which mats
	int &nFaceMatIndex)				//The length of the array
{
	bool r = false;
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return r;

	//find the directory of the obj file
	//the material file may use the same directory
	char dirpath[512];
	_findDirPath(fname, dirpath, 511);
	r = readOBJMesh(fp, dirpath, materials, vertices, normals, texCoords, facets, pFaceMatIndex, nFaceMatIndex);
	fclose(fp);
	return r;
}


inline void output_one_poly(std::ostream* out, const Vector3i &triangle, const Vector3i &texindex, const vector<Vector3f> &vtexcoords)
{
	const int vindex=0, nindex=0, tindex=0;
	*out << "f";
	for (int j=0; j<3; j++){
		int i0 = triangle[j];
		int i_p = i0;
		int i_n = i0;
		int i_t = texindex[j];
		if (vtexcoords.empty())
			*out << ' ' << i_p+vindex+1 << "//" << i_n+nindex+1;
		else
			*out << ' ' << i_p+vindex+1 << '/' << i_t+tindex+1 << '/' << i_n+nindex+1;
	}
	*out << '\n';
}


static void _exportOBJ(
		vector<OglMaterial>& materials, 
		const Vector3d *pVertex, const int nv, 
		const Vector3f *pNormal, const int nnorm, 
		const vector<Vector3f> &vtexcoords,
		const Vector3i *triangles, const int ntri,
		const Vector3i *pTexIndex,
		const std::string& objname, std::ostream* out, std::ostream* mtl, 
		const unsigned int faceMatIndex[32],//The material indice for polygons, each is encoded in (27bit position, 5bit matid)
		const int nFaceMatIndex				//The length of the index array, 
		)
{
	int i;
	*out << "g "<< objname << "\n";
	
	//export the first material in the obj
	if (mtl != NULL){ 
		const int nsize = materials.size();
		if (nsize>0)
			exportOBJMtl(mtl, &materials[0], nsize);
	}

	//export verices
	for (i=0; i<nv; i++){
		*out << "v " 
			 << std::fixed << pVertex[i][0] << ' ' 
			 << std::fixed << pVertex[i][1] << ' ' 
			 << std::fixed << pVertex[i][2] << '\n';
	}

	//export normal
	if (pNormal){
		for (int i=0; i<nnorm; i++){
			const Vector3f n = pNormal[i];
			*out << "vn " 
				 << std::fixed << n.x <<' '
				 << std::fixed << n.y <<' '
				 << std::fixed << n.z <<'\n';
		}
	}

	//export texture coord.
	if (!vtexcoords.empty()){
		const int nbt = vtexcoords.size();
		for (int i=0; i<nbt; i++){
			*out << "vt "
				 << std::fixed << vtexcoords[i][0] << ' '
				 << std::fixed << vtexcoords[i][1] << '\n';
		}
	}

	//export trianlges
	if (materials.size()==0 || faceMatIndex==NULL || nFaceMatIndex==0){
		Vector3i texindex(0,0,0);
		for (i=0; i<ntri; i++){
			const Vector3i &triangle = triangles[i];
			output_one_poly(out, triangle, texindex, vtexcoords);
		}
	}
	else{
		for (int j=0; j<nFaceMatIndex; j++){
			int x0, x1, matid;
			decodeMaterialIndex(j, faceMatIndex, nFaceMatIndex, ntri, x0, x1, matid);
			*out << "usemtl " << materials[matid].name << "\n";
			for (i=x0; i<x1; i++){
				const Vector3i &triangle = triangles[i];
				const Vector3i &texindex = pTexIndex[i];
				output_one_poly(out, triangle, texindex, vtexcoords);	
			}
		}

	}
	*out << std::endl;
}


bool writeOBJFile(
		const char *filename,				//output object file name 
		const std::string& objname,
		vector<OglMaterial>& materials, 
		const Vector3d *pVertex, const int nv, 
		const Vector3f *pNormal, const int nnorm, 
		const vector<Vector3f> &vtexcoords,
		const Vector3i *triangles, const int ntri,
		const Vector3i *pTexIndex,
		const unsigned int faceMatIndex[32],//The material indice for polygons, each is encoded in (27bit position, 5bit matid)
		const int nFaceMatIndex				//The length of the index array, 
	)
{
	bool r = true;
	std::string mtlname =filename; 
	mtlname += ".mtl";

	std::filebuf fb, fb2;
	fb.open(filename, ios::out);
	fb2.open(mtlname.c_str(), ios::out);
	std::ostream out(&fb);
	std::ostream mtl(&fb2);

	out << "mtllib " << mtlname << "\n";
	_exportOBJ(
		materials, 
		pVertex, nv, 
		pNormal, nnorm, vtexcoords,
		triangles, ntri, pTexIndex,
		objname, &out, &mtl, 
		faceMatIndex,	//The material indice for polygons, each is encoded in (27bit position, 5bit matid)
		nFaceMatIndex	//The length of the index array, 
	);

	fb.close();
	fb2.close();
	return r;
}