//FILE: sphere16.h

#ifndef __INC_SPHERE16_H__
#define __INC_SPHERE16_H__


#include <vector3f.h>
#include <vector3d.h>
#include <matrix3x3.h>


class CSphere16
{
private:
		CSphere16(void);
		~CSphere16();
		void _draw(const float r, const bool reverse_normal=false);
		void _draw(const float3x3& mat, const bool reverse_normal=false);
		int _initSphereMesh(void);

public:
		inline static CSphere16& getInstance(void)
		{
			static CSphere16 inst;
			return inst;
		}
		
		inline void glDraw(const Vector3f &pos, const float& r, const bool reverse_normal)
		{
			glPushMatrix();
				glTranslatef(pos.x, pos.y, pos.z);			
				_draw(r, reverse_normal);
			glPopMatrix();
		}

		inline void glDraw(const Vector3d &pos, const double& r, const bool reverse_normal)
		{
			const Vector3f posf(pos.x, pos.y, pos.z);
			const float rf = r;
			glDraw(posf, rf, reverse_normal);
		}

		inline void glDraw(const Vector3f &pos, const float3x3& mat, const bool reverse_normal)
		{
			glPushMatrix();
				glTranslatef(pos.x, pos.y, pos.z);			
				_draw(mat, reverse_normal);
			glPopMatrix();
		}

		inline void glDraw(const Vector3d &pos, const double3x3& mat, const bool reverse_normal)
		{
			const Vector3f posf(pos.x, pos.y, pos.z);
			float3x3 matf;
			for (int i=0; i<9; i++) matf.x[i]=mat.x[i];
			glDraw(posf, matf, reverse_normal);
		}
};


#endif