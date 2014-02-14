//FILE: stiffimage.h

#ifndef _INC_STIFFIMAGE_
#define _INC_STIFFIMAGE_

#include <winbmp_io.h>
#include "zsparse_matrix.h"

class CStiffImage
{
public:
	CStiffImage(const int nvert=4)
	{
		m_blockWidth = 32;
		m_ndim = nvert * 3;
		m_width = m_blockWidth * m_ndim;
		m_pRGBA = new unsigned char [m_width*m_width*4];
		_init();
	}

	~CStiffImage(void)
	{
		delete [] m_pRGBA;
	}

	void setBlockPixelColor(const int x, const int _y, unsigned char rgba[4])
	{
		const int y = m_ndim - 1 - _y;
		const int x0 = x*m_blockWidth; 
		const int x1 = x0+m_blockWidth; 
		const int y0 = y*m_blockWidth; 
		const int y1 = y0+m_blockWidth; 
		unsigned char red[4]={255, 0, 0, 255};

		for (int j=y0; j<y1; j++){
			for (int i=x0; i<x1; i++){
				const int ind = (j*m_width+i)*4;
				unsigned char *p = &m_pRGBA[ind];
				p[0]=rgba[0]; p[1]=rgba[1];
				p[2]=rgba[2]; p[3]=rgba[3];
			}	
		}
		{   
			int j=y0;
			for (int i=x0; i<x1; i++){
				const int ind = (j*m_width+i)*4;
				unsigned char *p = &m_pRGBA[ind];
				p[0]=red[0]; p[1]=red[1];
				p[2]=red[2]; p[3]=red[3];
			}	
		}
		{   
			int i=x0;
			for (int j=y0; j<y1; j++){
				const int ind = (j*m_width+i)*4;
				unsigned char *p = &m_pRGBA[ind];
				p[0]=red[0]; p[1]=red[1];
				p[2]=red[2]; p[3]=red[3];
			}	
		}

	}

	inline unsigned char *getColorBuffer(void)
	{
		return m_pRGBA;
	}

	inline int getWidth(void) const
	{
		return m_width;
	}

	inline int getHeight(void) const
	{
		return m_width;
	}

	inline int getDimm(void) const
	{
		return m_ndim;
	}

private:
	int m_width;
	int m_ndim;
	int m_blockWidth;
	unsigned char *m_pRGBA;

	void _init()
	{
		unsigned char rgba[4]={0,0,0,0};
		for (int x=0; x<m_ndim; x++)
			for (int y=0; y<m_ndim; y++)
				setBlockPixelColor(x, y, rgba);
	}

};


inline void saveImage(const double& scalefactor, CStiffImage& img, const int ii, const int jj, double3x3 *e)
{
	static unsigned char bgcolor[4]={0, 0, 0, 255};
	const int i0 = ii*3;
	const int j0 = jj*3;
	for (int x=0, index=0; x<3; x++){
		for (int y=0; y<3; y++, index++){
			const int u=x+i0, v=y+j0;
			double val=0;
			if (e) val= fabs(e->x[index]);
			if (val==0){
				img.setBlockPixelColor(u, v, bgcolor);
			}
			else{
				val*=scalefactor*255;
				if (val>255.0) val=255.0;
				unsigned char color[4];
				color[0]=color[1]=color[2]=(unsigned char)val;
				color[3]=255;
				img.setBlockPixelColor(u, v, color);
			}			
		}
	}
}


inline void saveStiffnessToImage(const double scalefactor, const char *fname, CSparseMatrix33 *pMatRod)
{
	int nx = pMatRod->size();
	if (nx>20) return;	//too big to save
	CStiffImage image(nx);
	for (int i=0; i<nx; i++){
		for (int j=0; j<nx; j++){
			double3x3 *e1 = pMatRod->getMatrixElement(i, j);
			saveImage(scalefactor, image, i, j, e1);
		}
	}

	CBitmap rodbmp;
	unsigned char *pbuff1 = image.getColorBuffer();
	int w = image.getWidth();
	int h = image.getHeight();
	unsigned int redmask   = 0x000000ff;
	unsigned int greenmask = 0x0000ff00;
	unsigned int bluemask  = 0x00ff0000;
	unsigned int alphamask = 0xff000000;
	rodbmp.SetBits(pbuff1, w, h, redmask, greenmask, bluemask, alphamask);
	rodbmp.Save(fname, 32);
}

#endif