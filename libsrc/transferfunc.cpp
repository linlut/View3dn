//FILE: transferfunc.cpp
#include <transferfunc.h>
#include <assert.h>
#include <stdio.h>
#include <vector>

using namespace std;

static Vector4f color1[]={
	Vector4f(-100, 0, 1, 0), 
	Vector4f(0.15, 0, 1, 0),
	Vector4f(0.225, 1, 0, 1),
	Vector4f(0.3, 1, 0, 0),
	Vector4f(0.6, 1, 1, 0),
	Vector4f(1.5, 1, 1, 1),
	Vector4f(500, 1, 1, 1)
};

static Vector2f opacity1[]={
	Vector2f(-500, 0.2),
	Vector2f(.15, .2),
	Vector2f(.2, .4),
	Vector2f(.45, .4), 
	Vector2f(1, .6),
	Vector2f(1.5, 1),
	Vector2f(500, 1)
};


static Vector3f interpolatedColor(Vector4f color[], const int len, float x)
{
	Vector3f c(0,0,0);
	if (x<=color[0].x){
		c.x = color[0].y;
		c.y = color[0].z;
		c.z = color[0].w;
		return c;
	}
	const int len1 = len -1;
	if (x>=color[len1].x){
		c.x = color[len1].y;
		c.y = color[len1].z;
		c.z = color[len1].w;
		return c;
	}
	for (int i=1; i<len; i++){
		if (x<=color[i].x){
			const int a = i-1;
			const int b = i;
			Vector3f *colora = (Vector3f*)(&color[a].y);
			Vector3f *colorb = (Vector3f*)(&color[b].y);
			float t = (x-color[a].x)/(color[b].x-color[a].x);
			c = (*colora)*(1-t)+(*colorb)*t;
			break;
		}
	}
	return c;
}


static float interpolatedOpacity(Vector2f color[], const int len, float x)
{
	float c = 0;
	if (x<=color[0].x){
		c = color[0].y;
		return c;
	}
	const int len1 = len -1;
	if (x>=color[len1].x){
		c = color[len1].y;
		return c;
	}
	for (int i=1; i<len; i++){
		if (x<=color[i].x){
			const int a = i-1;
			const int b = i;
			float colora = (color[a].y);
			float colorb = (color[b].y);
			float t = (x-color[a].x)/(color[b].x-color[a].x);
			c = colora*(1-t) + colorb*t;
			break;
		}
	}
	return c;
}


void CTransferFunc::setSpecialTransferFuncB(const float minval, const float maxval)
{
	const int colorlen = sizeof(color1)/sizeof(Vector4f);
	const int opalen = sizeof(opacity1)/sizeof(Vector2f);
	const float fstep = (maxval - minval)/255.0;
	unsigned char *p = &m_colorTable[0][0];
	const float SA = 255.0/20;
	for (int i=0; i<256; i++, p+=4){
		const float isoval = minval + i*fstep;
		Vector3f c = interpolatedColor(color1, colorlen, isoval);
		float a = interpolatedOpacity(opacity1, opalen, isoval);
		c*=255.0;
		a*=SA;
		p[0] = c.x;
		p[1] = c.y;
		p[2] = c.z;
		p[3] = a;
	}
}


inline void _setFEMStylePalette(unsigned char m_texture1D[256][4])
{
	int i, x, y, base;
	//set up the palette for mapping 
	for (i=0; i<256; i++){
		unsigned char *p = m_texture1D[i];
		//p[0]=p[1]=p[2]=0, p[3]=32;
		p[0]=p[1]=p[2]=0, p[3]=4;
	}

	x = 2, y = 1;
	for (i=0; i<64; i++){
		base = i + 0;
		m_texture1D[base][x] = 255;
		m_texture1D[base][y] = i*4;
		m_texture1D[base][0] = 0;
	}
	for (i=0; i<64; i++){
		base = i + 64;
		m_texture1D[base][x] = 255-i*4;
		m_texture1D[base][y] = 255;
	}
	x = 1, y = 0;
	for (i=0; i<64; i++){
		base = i + 128;
		m_texture1D[base][x] = 255;
		m_texture1D[base][y] = i*4;
	}
	for (i=0; i<64; i++){
		base = i + 192;
		m_texture1D[base][x] = 255-i*4;
		m_texture1D[base][y] = 255;
	}
}

void CTransferFunc::setSpecialTransferFuncA(void)
{
	_setFEMStylePalette(m_colorTable);
}

void CTransferFunc::installTransferFuncOnGPU(void)
{
	{
		/* func not supported by NVIDIA yet
		// download color table for pre-classification
		glColorTableEXT( GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGBA8, 256 * 4,
			GL_RGBA, GL_UNSIGNED_BYTE, m_pTransferFunc);
		*/
		// generate texture id
		glGenTextures(1, &m_nTransFuncTexID);
		assert(m_nTransFuncTexID!=0);

		glBindTexture( GL_TEXTURE_1D, m_nTransFuncTexID );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// install 1D transfer function texture for post-classificati
		unsigned char texbuff[(256+2)*4];
		memcpy(texbuff+4, m_colorTable, 256*4);
		texbuff[0]=texbuff[4];	texbuff[1]=texbuff[5];  
		texbuff[2]=texbuff[6];  texbuff[3]=texbuff[7];
		texbuff[1028]=texbuff[1024]; texbuff[1029]=texbuff[1025]; 
		texbuff[1030]=texbuff[1026]; texbuff[1031]=texbuff[1027];
		glTexImage1D( GL_TEXTURE_1D, 0, 4, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, texbuff);
	}
}

	
void CTransferFunc::setIsosurfaceTransferFunc(const int isovalue, const Vector3i& clr)
{
	int i;
	for (i=0; i<isovalue; i++){
		unsigned char *p=m_colorTable[i];
		//p[0]=clr.x; p[1]=clr.y; p[2]=clr.z;
		p[0]=p[1]=p[2]=0;
		p[3]=0;			
	}
	for (i=isovalue; i<256; i++){
		unsigned char *p=m_colorTable[i];
		p[0]=clr.x; p[1]=clr.y; p[2]=clr.z;
		p[3]=255;			
	}
}


class CTFControlPoint
{
public:
	float x, y;
	Vector3f color;
	float alpha;

	CTFControlPoint(void)
	{
	}

	void read(FILE *fp)
	{
		fscanf(fp, "%f %f", &x, &y);
		float r, g, b;
		fscanf(fp, "%f %f %f", &r, &g, &b);
		color = Vector3f(r, g, b);
		fscanf(fp, "%f", &alpha);
	}
};

class CTFControlPointA
{
public:
	float x, y;

	CTFControlPointA(void)
	{
	}

	void read(FILE*fp)
	{
		fscanf(fp, "%f %f", &x, &y);
	}
};

static void getDiscretizedTransFunc(vector<CTFControlPoint>&CP,  unsigned char p[256*4])
{
	float r0, g0, b0, r1, g1, b1;
	unsigned char *chptr;
	const int nsize = CP.size();
	for (int i=0; i<nsize-1; i++){
		const CTFControlPoint& A= CP[i];
		const CTFControlPoint& B= CP[i+1];
		const int x0 = A.x*255;
		const int x1 = B.x*255;
		const float det = 1.0f/(fabs(double(x1-x0)) + 1e-10f);
		r0 = A.color.x;
		g0 = A.color.y;
		b0 = A.color.z;
		r1 = B.color.x;
		g1 = B.color.y;
		b1 = B.color.z;
		Vector3f Ca=Vector3f(r0,g0,b0)*A.alpha;
		Vector3f Cb=Vector3f(r1,g1,b1)*B.alpha;
		chptr = p+x0*4;
		for (int j=x0; j<=x1; j++,chptr+=4){
			const float t = (j-x0)*det;
			Vector3f clr = (1-t)*Ca+t*Cb;
			chptr[0] = clr.x;
			chptr[1] = clr.y;
			chptr[2] = clr.z;
		}
	}
}

static void getDiscretizedTransFuncAlpha(vector<CTFControlPointA> CP, unsigned char p[256*4])
{
	unsigned char *chptr;
	const int nsize = CP.size();
	for (int i=0; i<nsize-1; i++){
		const CTFControlPointA& A= CP[i];
		const CTFControlPointA& B= CP[i+1];
		const int x0 = A.x*255;
		const int x1 = B.x*255;
		const float det = 1.0f/(abs(x1-x0) + 1e-16f);
		const float Ca = (1.0f-A.y)*255;
		const float Cb = (1.0f-B.y)*255;
		chptr = p+x0*4;
		for (int j=x0; j<=x1; j++,chptr+=4){
			const float t = (j-x0)*det;
			chptr[3] = ((1-t)*Ca+t*Cb);
		}
	}
}

bool CTransferFunc::load(const char *fname)
{
	vector<CTFControlPoint> colorcp;
	vector<CTFControlPointA> alphacp;
		
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return false;
	//load the colors;
	int i, np;
	fscanf(fp, "%d", &np);
	for (i=0; i<np; i++){
		CTFControlPoint pt;
		pt.read(fp);
		colorcp.push_back(pt);
	}
	fscanf(fp, "%d", &np);
	for (i=0; i<np; i++){
		CTFControlPointA pt;
		pt.read(fp);
		alphacp.push_back(pt);
	}
	fclose(fp);
	
	//convert into discretized form;
	getDiscretizedTransFunc(colorcp, &m_colorTable[0][0]);
	getDiscretizedTransFuncAlpha(alphacp, &m_colorTable[0][0]);
	return true;
}
