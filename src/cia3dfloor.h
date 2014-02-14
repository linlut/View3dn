//file: cia3dfloor.h


#ifndef __INC_CIA_FLOOR_H__
#define __INC_CIA_FLOOR_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include "object3d.h"


class CFloor
{
private:
	AxisAlignedBox m_Box;
	bool m_bDrawX;
	bool m_bDraw_X;
	bool m_bDrawY;
	bool m_bDraw_Y;
	bool m_bDrawZ;
	bool m_bDraw_Z;

	void _glDrawQuad(Vector3f v[3], const Vector3f& norm, const int nu, const int nv)
	{
		int i,j;
		Vector3f du = (v[1] - v[0])/nu;
		Vector3f dv = (v[2] - v[0])/nv;
		Vector3f x0, x1, y0, y1;

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);

		glNormal3fv(&norm.x);
		for (j=0; j<nv; j++){
			for (i=0; i<nu; i++){
				x0 = v[0]+ i*du + j*dv;
				x1 = x0+du;
				y0 = x0+dv;
				y1 = y0+du;
				if ((i+j)&0x1)
					glColor3f(0,0.90f,0);
				else
					glColor3f(0.8f,0.8f,0);
				glBegin(GL_QUADS);
					glVertex3fv(&x0.x);
					glVertex3fv(&x1.x);
					glVertex3fv(&y1.x);
					glVertex3fv(&y0.x);
				glEnd();
			}
		}
	}

public:
	CFloor(void)
	{
		m_bDrawX = false;
		m_bDraw_X = false;
		m_bDrawY = false;
		m_bDraw_Y = false;
		m_bDrawZ = false;
		m_bDraw_Z = false;
	}

	void SetBoundingBox(const AxisAlignedBox& box)
	{
		m_Box = box;
	}

	void SetDrawX(const bool f)
	{
		m_bDrawX = f;
	}
	void SetDrawY(const bool f)
	{
		m_bDrawY = f;
	}
	void SetDrawZ(const bool f)
	{
		m_bDrawZ = f;
	}
	void SetDraw_X(const bool f)
	{
		m_bDraw_X = f;
	}
	void SetDraw_Y(const bool f)
	{
		m_bDraw_Y = f;
	}
	void SetDraw_Z(const bool f)
	{
		m_bDraw_Z = f;
	}

	void glDraw(void)
	{
		Vector3f v[3], origin;
		Vector3f norm;
		const int nu = 12;  
		const int nv = 12;

		Vector3d offset = m_Box.maxp - m_Box.minp;
		const double dist = (offset.x+offset.y+offset.z)*0.333*0.010;

		Vector3d center = (m_Box.maxp + m_Box.minp)*0.5;
		double scale = _MAX3_(offset.x, offset.y, offset.z);

		if (m_bDrawZ){
			origin.x = center.x - scale;
			origin.y = center.y - scale;
			origin.z = m_Box.minp.z;
			v[0] = origin;
			v[0].z -= dist;
			v[1] = v[2]= v[0];
			v[1].x = center.x + scale;
			v[2].y = center.y + scale;		
			norm = Vector3f(0,0,1);
			_glDrawQuad(v, norm, nu, nv);
		}
	}

};

#endif