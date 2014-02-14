//CVolume3D class
#include <zvolume.h>

static float *convertByteToFloat(const unsigned char *pbyte, const int nx, const int ny, const int nz)
{
	const float K=1.0f/255;
	int i, nsize=nx*ny*nz;
	float *p = new float[nsize];
	assert(p!=NULL);
	for (i=0; i<nsize; i++)
		p[i]=pbyte[i]*K;
	return p;
}

static inline void readFloat3(FILE *fp, Vector3f & v, const Vector3f& upright)
{
	fscanf(fp, "%f %f %f", &v.x, &v.y, &v.z);
	if (v.x<0) v.x=0;
	if (v.x>upright.x) v.x=upright.x;
	if (v.y<0) v.y=0;
	if (v.y>upright.y) v.y=upright.y;
	if (v.z<0) v.z=0;
	if (v.z>upright.z) v.z=upright.z;
}

static bool getFileInfo(const char *fname, int &m_nx, int &m_ny, int &m_nz, int &bpv, Vector3f brick[][2], int &nbrick)
{
	const int slen = strlen(fname);
	bool r=true;
	char fn[512];
	nbrick = 0;

	//read the infor file first, for volume def
	strcpy(fn, fname);
	fn[slen]='.', fn[slen+1]='i', fn[slen+2]='n', fn[slen+3]='f', fn[slen+4]='o', fn[slen+5]=0; 
	FILE *fp = fopen(fn, "r");
	if (fp){
		fscanf(fp, "%d %d %d %d", &m_nx, &m_ny, &m_nz, &bpv);
		fscanf(fp, "%d", &nbrick);
		if (nbrick<=0) goto RT_POS;
		const Vector3f upright(m_nx-1, m_ny-1, m_nz-1);
		for (int i=0; i<nbrick; i++){
			readFloat3(fp, brick[i][0], upright);
			readFloat3(fp, brick[i][1], upright);
		}
		fclose(fp);
		return true;
	}

	bpv = 1;
	if (strstr(fname, "neghip.raw") || strstr(fname, "fuel.raw") )
		m_nx=m_ny=m_nz=64;
	else if (strstr(fname, "engine.raw"))
		m_nx=m_ny=256, m_nz=128;
	else if (strstr(fname, "foot.raw"))
		m_nx=m_ny=256, m_nz=256;
	else if (strstr(fname, "hydrogenAtom.raw"))
		m_nx=m_ny=m_nz=128;
	else if (strstr(fname, "bonsai.raw"))
		m_nx=m_ny=m_nz=256;
	else if (strstr(fname, "skull.raw"))
		m_nx=m_ny=m_nz=256;
	else{
		r = false;
	}

RT_POS:
	if (r) nbrick=1;
	brick[0][0]=Vector3f(0,0,0);
	brick[0][1]=Vector3f(m_nx-1, m_ny-1, m_nz-1);
	return r;
}


void CVolume3D::_free(void)
{
	SafeDeleteArray(m_pDensityVolume);
	SafeDeleteArray(m_pGradientVolume);
	m_nx=m_ny=m_nz=0;

	//===========================
	uninstallDensityVolumeOnGPU();
	uninstallGradientVolumeOnGPU();
}


bool CVolume3D::_loadRAWFile(const char *fname)
{
	int bpv;	//bytes per voxel
	if (!getFileInfo(fname, m_nx, m_ny, m_nz, bpv, m_vBrickBuffer, m_nBrick)){
		printf("Cannot find the info file associated with %s.\n", fname);
		return false;
	}

	m_nxy = m_nx*m_ny;	
	FILE *fp = fopen(fname, _RB_);
	if (fp==NULL) return false;
	const int nsize = m_nx*m_ny*m_nz;
	m_pDensityVolume = (float*)(new unsigned char [nsize*bpv]);
	fread(m_pDensityVolume, bpv*m_nx, m_ny*m_nz, fp);
	fclose(fp);

	switch(bpv){
		case 1:
			{
				float *p=convertByteToFloat((unsigned char*)m_pDensityVolume, m_nx, m_ny, m_nz);
				delete [] m_pDensityVolume;
				m_pDensityVolume =p;
				break;
			}
		case 4:
			break;
	}
	m_nDensityVolumeType = GL_FLOAT;
	return true;
}

bool CVolume3D::_loadSLCFile(const char *fname)
{
	//read the box splitter
	int xx, yy, zz, bpv;
	getFileInfo(fname, xx, yy, zz, bpv, m_vBrickBuffer, m_nBrick);

	extern unsigned char * 
	LoadVolVisDataset(const char* datasetFileName, unsigned int* x, unsigned int* y, unsigned int* z);
	unsigned int nx, ny, nz;
	unsigned char *p = LoadVolVisDataset(fname, &nx, &ny, &nz);
	if (p==NULL) return false;
	m_pDensityVolume=convertByteToFloat(p, nx, ny, nz);
	delete [] p;

	m_nx = nx, m_ny=ny, m_nz=nz;
	m_nxy = m_nx*m_ny;
	m_nDensityVolumeType = GL_FLOAT;	
	return true;
}

bool CVolume3D::load(const char *fname)
{
	bool r=false;
	int slen = strlen(fname);
	const char *fext = &fname[slen-4];
	if (strcmp(fext,".raw")==0 || strcmp(fext,".RAW")==0){
		r = _loadRAWFile(fname);
	}
	else if (strcmp(fext,".slc")==0 || strcmp(fext,".SLC")==0){
		r = _loadSLCFile(fname);
	}
	else{
	}

	return r;
}


void CVolume3D::_scaleNormalizedGradients(const float K)
{
	int i;
	const int nsize = m_nx*m_ny*m_nz;

	//find the max grad magnitude and normalize the whole volume;
	float maxg = 0;
	for (i=0; i<nsize; i++){
		const float x= m_pGradientVolume[i].w;
		if (x>maxg) maxg=x;
	}
	maxg+=1e-20f;
	float maxg1 = K/maxg;
	for (i=0; i<nsize; i++)
		m_pGradientVolume[i].w*=maxg1;	
}


void CVolume3D::computeGradient(void)
{
	int i, j, k;

	SafeDelete(m_pGradientVolume);
	m_nxy = m_nx*m_ny;
	const int nsize = m_nxy*m_nz;
	m_pGradientVolume =new Vector4f[nsize];
	if (m_pGradientVolume==NULL){
		unsigned int maxsize = nsize*sizeof(Vector4f);
		do{
			char *pchar = (char*)malloc(maxsize);
			if (pchar!=NULL){
				printf("Max malloc size is %d!\n", maxsize);
				break;
			}
			else
				maxsize-=1024;
			if (maxsize<16) maxsize=16;
		}while(true);
		printf("Not enough memroy!\n");
		exit(0);
	}
	memset(m_pGradientVolume,0, sizeof(Vector4f)*nsize);

	const Vector3f h(0.5f, 0.5f, 0.5f);
	for (k=1; k<m_nz-1; k++){
		for (j=1; j<m_ny-1; j++){
			Vector4f *p = m_pGradientVolume + (k*m_nxy+j*m_nx);
			for (i=1; i<m_nx-1; i++){
				Vector4f *v4 = &p[i];
				centralDifference(i, j, k, *v4);
				Vector3f *v3 = (Vector3f*) v4;
				(*v3) = (*v3)*0.5f+h;
			}
		}
	}

	//scale to max gradient magnitude
	_scaleNormalizedGradients(1);
}


#define DataType int
void CVolume3D::_computeSobelGradients(float *gradients, const int stride, int *sizes, const int dataType, const float gsliceDists[3])
{
    int idz, idy, idx;
    float *gp=gradients;
    const float weights[3][3][3][3] = {
        {{{-1, -3, -1}, {-3, -6, -3}, {-1, -3, -1}},
         {{ 0,  0,  0}, { 0,  0,  0}, { 0,  0,  0}},
         {{ 1,  3,  1}, { 3,  6,  3}, { 1,  3,  1}}},
        {{{-1, -3, -1}, { 0,  0,  0}, { 1,  3,  1}},
         {{-3, -6, -3}, { 0,  0,  0}, { 3,  6,  3}},
         {{-1, -3, -1}, { 0,  0,  0}, { 1,  3,  1}}},
        {{{-1,  0,  1}, {-3,  0,  3}, {-1,  0,  1}},
         {{-3,  0,  3}, {-6,  0,  6}, {-3,  0,  3}},
         {{-1,  0,  1}, {-3,  0,  3}, {-1,  0,  1}}}
    };

	fprintf(stderr, "computing gradients using SOBEL filter... may take a while\n");
    for (idz = 0; idz < sizes[2]; idz++) {
        for (idy = 0; idy < sizes[1]; idy++) {
            for (idx = 0; idx < sizes[0]; idx++) {
                if ((idx > 0 && idx < sizes[0] - 1) &&
                    (idy > 0 && idy < sizes[1] - 1) &&
                    (idz > 0 && idz < sizes[2] - 1)) {
					sobelFilter3D(idx, idy, idz, weights, gsliceDists, gp);
                } else {
                    /* X-direction */
                    if (idx < 1) {
                        gp[0] = (getVoxel(idx + 1, idy, idz, dataType) - getVoxel(idx, idy, idz, dataType))* (gsliceDists[0]);
                    } else {
                        gp[0] = (getVoxel(idx, idy, idz, dataType) - getVoxel(idx - 1, idy, idz, dataType))* (gsliceDists[0]);
                    }
                    /* Y-direction */
                    if (idy < 1) {
                        gp[1] = (getVoxel(idx, idy + 1, idz, dataType) - getVoxel(idx, idy, idz, dataType))* (gsliceDists[1]);
                    } else {
                        gp[1] = (getVoxel(idx, idy, idz, dataType) - getVoxel(idx, idy - 1, idz, dataType))* (gsliceDists[1]);
                    }
                    /* Z-direction */
                    if (idz < 1) {
                        gp[2] = (getVoxel(idx, idy, idz + 1, dataType) - getVoxel(idx, idy, idz, dataType))* (gsliceDists[2]);
                    } else {
                        gp[2] = (getVoxel(idx, idy, idz, dataType) - getVoxel(idx, idy, idz - 1, dataType))* (gsliceDists[2]);
                    }
                }
                gp += stride;
            }
        }
    }
}



void CVolume3D::computeGradientSobel(void)
{
	SafeDelete(m_pGradientVolume);
	m_nxy = m_nx*m_ny;
	const int nsize = m_nxy*m_nz;
	m_pGradientVolume =new Vector4f[nsize];
	memset(m_pGradientVolume,0, sizeof(Vector4f)*nsize);

	//compute gradients
	int sizes[3]={m_nx, m_ny, m_nz};
	float gsliceDists[3] = {1,1,1};
	_computeSobelGradients((float*)m_pGradientVolume, sizeof(Vector4f)/sizeof(float), sizes, 0, gsliceDists);

	//scale to max gradient magnitude
	_scaleNormalizedGradients(1);
}


static inline float dist2Density(const float dist, const float r)
{
	const float w = 4;
	const float w1 = 1.0f/w;
	float d = dist - r;
	if (d>w) return 0;
	if (d<-w) return 1;
	float denf = (-d*w1+1)*0.50f;
	return denf;
}

void CVolume3D::createSphereVolume(const int nx, const float r)
{
	int i, j, k;

	//realeas old space
	_free();
	m_nx = m_ny = m_nz = nx;
	m_nDensityVolumeType = GL_FLOAT;

	//density volume allocation
	const int nsize = nx*nx*nx;
	m_pDensityVolume = new float [nsize];
	assert(m_pDensityVolume!=NULL);
	float *p = m_pDensityVolume;

	m_pGradientVolume = new Vector4f[nsize];
	assert(m_pGradientVolume!=NULL);
	Vector4f *pgrad = m_pGradientVolume;

	//assign dist val using non-binary voxelization
	const Vector3f h(0.5f, 0.5f, 0.5f);
	const float cc = (nx-1)*0.5f;
	const Vector3f center(cc, cc, cc);
	for (k=0; k<nx; k++){
		for (j=0; j<nx; j++){
			for (i=0; i<nx; i++, p++, pgrad++){
				Vector3f v(i, j, k);
				Vector3f dist=v-center;
				float d2 = DotProd(dist, dist);
				const float d = (float)sqrt(d2);
				const float d1 = 0.5f/(d+1e-6f);    //make sure none-zero
				float u=dist2Density(d, r);
				*p  = u;
				const Vector3f N=dist*d1+h;
				pgrad->x = N.x;
				pgrad->y = N.y;
				pgrad->z = N.z;
				pgrad->w=1;
			}
		}
	}
}


void CVolume3D::createCubeVolume(const int nx, const float w)
{
	//realeas old space
	_free();
/*
	const int nsize = nx*nx*nx;
	m_pDensityVolume = new unsigned char [nsize];
	for 
*/
}


void CVolume3D::installDensityVolumeOnGPU(void)
{
	// generate texture id
	glGenTextures(1, &m_nDensTexID);

	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, m_nDensTexID);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// download 3D volume texture for pre-classification
	glTexImage3D( GL_TEXTURE_3D, 0, 1,
				  m_nx, m_ny, m_nz, 0, 
				  GL_LUMINANCE, 
				  m_nDensityVolumeType, m_pDensityVolume );
}

void CVolume3D::installGradientVolumeOnGPU(void)
{
	// generate texture id
	glGenTextures(1, &m_nGradTexID);

	// bind 3D texture target
	glBindTexture(GL_TEXTURE_3D, m_nGradTexID);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	// download 3D volume texture for pre-classification
	glTexImage3D( GL_TEXTURE_3D, 0, 4,
				  m_nx, m_ny, m_nz, 0, 
				  GL_RGBA, GL_FLOAT, m_pGradientVolume );
}


void CVolume3D::uninstallDensityVolumeOnGPU(void)
{
	if (m_nDensTexID)
		glDeleteTextures(1, &m_nDensTexID);
	m_nDensTexID = 0;
}

void CVolume3D::uninstallGradientVolumeOnGPU(void)
{
	if (m_nGradTexID)
		glDeleteTextures(1, &m_nGradTexID);
	m_nGradTexID = 0;
}



//===============================================
const int VOLUME_BLOCK_SIZE = (8);

CVolumeBlockBuffer::CVolumeBlockBuffer():
	mBlocks()
	,m_nx(0)
	,m_ny(0)
	,m_nz(0)
{
}

CVolumeBlockBuffer::~CVolumeBlockBuffer()
{
	m_nx = m_ny = m_nz = 0;
	mBlocks.clear();
}

inline float readVoxel(const float*p, int nx, int ny, int nz, int i, int j, int k)
{
	const int index = k*nx*ny + j*nx + i;
	return p[index];
}

void readBlock(int x, int y, int z, 
	const float *pVoxels, int nx, int ny, int nz, 
	float &vmin, float &vmax)
{
	int x0 = x*VOLUME_BLOCK_SIZE; 
	int x1 = x0+VOLUME_BLOCK_SIZE; 
	if (x1>nx-1) x1=nx-1;
	int y0 = y*VOLUME_BLOCK_SIZE; 
	int y1 = y0+VOLUME_BLOCK_SIZE; 
	if (y1>ny-1) y1=ny-1;
	int z0 = z*VOLUME_BLOCK_SIZE; 
	int z1 = z0+VOLUME_BLOCK_SIZE; 
	if (z1>nz-1) z1=nz-1;

	const int nxy = nx*ny;
	vmin = 1e36f;
	vmax = -vmin;
	for (int k=z0; k<=z1; k++){
		const int kz = k*nxy;
		for (int j=y0; j<=y1; j++){
			const int base = j*nx + kz;
			for (int i=x0; i<=x1; i++){
				const float vox = pVoxels[i + base];
				//float vox = readVoxel(pVoxels, nx, ny, nz, i, j, k);
				if (vox<vmin) vmin = vox;
				if (vox>vmax) vmax = vox;
			}
		}
	}
}

void CVolumeBlockBuffer::initBlocks(const float *pVoxels, int nx, int ny, int nz)
{
	const int delta = VOLUME_BLOCK_SIZE-2;
	m_nx = (nx+delta) / VOLUME_BLOCK_SIZE;
	m_ny = (ny+delta) / VOLUME_BLOCK_SIZE;
	m_nz = (nz+delta) / VOLUME_BLOCK_SIZE;
	const size_t blocksize = m_nx*m_ny*m_nz;
	mBlocks.resize(blocksize);

	int c = 0;
	Vector2f r;
	for (int k=0; k<m_nz; k++){
		for (int j=0; j<m_ny; j++){
			for (int i=0; i<m_nx; i++,c++){
				readBlock(i, j, k, pVoxels, nx, ny, nz, r.x, r.y);
				mBlocks[c] = r;
			}
		}
	}
}

static void extractBlock(int x, int y, int z,
	const float *pVoxels, int nx, int ny, int nz,
	const float isoval[], 
	const int ids[],
	int isoval_count,
	std::vector<int>& xcount, 
	std::vector<int>& ycount, 
	std::vector<int>& zcount, 
	std::vector<unsigned int>& voxels)	//voxels are stored in a compressed format 4-1-9-9-9
{
	int x0 = x*VOLUME_BLOCK_SIZE; 
	int x1 = x0+VOLUME_BLOCK_SIZE; 
	if (x1>nx-1) x1=nx-1;
	int y0 = y*VOLUME_BLOCK_SIZE; 
	int y1 = y0+VOLUME_BLOCK_SIZE; 
	if (y1>ny-1) y1=ny-1;
	int z0 = z*VOLUME_BLOCK_SIZE; 
	int z1 = z0+VOLUME_BLOCK_SIZE; 
	if (z1>nz-1) z1=nz-1;

	const int nxy = nx*ny;
	for (int k=z0; k<z1; k++){
		const int kz = k*nxy;
		for (int j=y0; j<y1; j++){
			const int base = j*nx + kz;
			for (int i=x0; i<x1; i++){
				const int b0 = i+base;
				const float v0 = pVoxels[b0];
				const float v1 = pVoxels[b0+1];
				const float v2 = pVoxels[b0+nx];
				const float v3 = pVoxels[b0+nx+1];
				const int b1 = b0+nxy;
				const float v4 = pVoxels[b1];
				const float v5 = pVoxels[b1+1];
				const float v6 = pVoxels[b1+nx];
				const float v7 = pVoxels[b1+nx+1];
				unsigned int vox = 0;
				for (int c=0; c<isoval_count; c++){
					const float iso = isoval[c];
					const int id = ids[c];
					int sum;
					sum =  int(v0>iso); sum += int(v1>iso);
					sum += int(v2>iso); sum += int(v3>iso);
					sum += int(v4>iso); sum += int(v5>iso);
					sum += int(v6>iso); sum += int(v7>iso);
					if (sum==0 || sum==8) continue;
					vox |= (1U<<id);
				}
				
				if (vox != 0){
					vox <<= 28;
					vox |= i+(j<<9)+(k<<18);
					voxels.push_back(vox);
					++xcount[i]; ++ycount[j]; ++zcount[k];
				}
			}
		}
	}
}

void CVolumeBlockBuffer::extractIsoVoxels(
	const float *pVoxels, int nx, int ny, int nz, const float isoval[4],
	std::vector<int>& xcount, 
	std::vector<int>& ycount, 
	std::vector<int>& zcount, 
	std::vector<unsigned int>& voxels)	//voxels are stored in a compact tuple <short,short>
										//where a 32-bit int's bit arrangement <i0,i1,i2,i3,x10,y10,z10>
{
	xcount.resize(nx);
	ycount.resize(ny);
	zcount.resize(nz);
	for (int i=0; i<nx; i++) xcount[i]=0;
	for (int i=0; i<ny; i++) ycount[i]=0;
	for (int i=0; i<nz; i++) zcount[i]=0;
	voxels.clear();
	voxels.reserve((nx*ny*nz/30)+10000);

	int bcount = 0;
	for (int k=0; k<m_nz; k++){
		for (int j=0; j<m_ny; j++){
			for (int i=0; i<m_nx; i++,bcount++){
				const Vector2f voxrange = mBlocks[bcount];
				float isoval2[4];
				int ids[4], s=0;
				for (int u=0; u<4; u++){
					const float iso = isoval[u];
					if (voxrange.x<iso && iso<voxrange.y){ //possible intersect isosurface
						isoval2[s] = iso; ids[s] = u;
						++s;
					}
				}
				if (s>0){//
					extractBlock(i, j, k, pVoxels, nx, ny, nz,
						isoval2, ids, s,
						xcount, ycount, zcount, voxels);
				}
			}
		}
	}
}

inline void drawCube(const Vector3f & pos)
{
	Vector3f v[8];
	v[0] = pos;
	v[1] = pos; v[1].x+=1;
	v[2] = pos; v[2].y+=1;
	v[3] = pos; v[3].x+=1; v[3].y+=1;

	Vector3f pos1 = pos; pos1.z+=1;
	v[4] = pos1; 
	v[5] = pos1; v[5].x+=1;
	v[6] = pos1; v[6].y+=1;
	v[7] = pos1; v[7].x+=1; v[7].y+=1;

	glBegin(GL_LINES);
		glVertex3fv(&v[0].x);
		glVertex3fv(&v[1].x);
		glVertex3fv(&v[0].x);
		glVertex3fv(&v[2].x);
		glVertex3fv(&v[3].x);
		glVertex3fv(&v[2].x);
		glVertex3fv(&v[3].x);
		glVertex3fv(&v[1].x);

		glVertex3fv(&v[4].x);
		glVertex3fv(&v[5].x);
		glVertex3fv(&v[4].x);
		glVertex3fv(&v[6].x);
		glVertex3fv(&v[7].x);
		glVertex3fv(&v[6].x);
		glVertex3fv(&v[7].x);
		glVertex3fv(&v[5].x);

		glVertex3fv(&v[0].x);
		glVertex3fv(&v[4].x);
		glVertex3fv(&v[1].x);
		glVertex3fv(&v[5].x);
		glVertex3fv(&v[2].x);
		glVertex3fv(&v[6].x);
		glVertex3fv(&v[3].x);
		glVertex3fv(&v[7].x);
	glEnd();
}


void drawCubes(const Vector3i & lowleft, const Vector3i & upright)
{
	for (int i=lowleft.x; i<=upright.x; i++){
		for (int j=lowleft.y; j<=upright.y; j++){
			for (int k=lowleft.z; k<=upright.z; k++){
				drawCube(Vector3f(i, j, k));
			}
		}
	}
}