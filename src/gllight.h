#ifndef _INC_GL_LIGHT_ZN_H_
#define _INC_GL_LIGHT_ZN_H_

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <vectorall.h>


class CPointLight
{

public:
	bool m_bEnable;			//enable the light or not
	Vector3f m_position;	//position
	Vector3f m_ambient;		//ambient color
	Vector3f m_diffuse;		//diffuse color
	Vector3f m_specular;	//diffuse color

private:
	void init(const Vector3f& pos, const Vector3f& color, const float scale);

public:
	
	CPointLight(void);

	CPointLight(const Vector3f& pos);

	CPointLight(const Vector3f& pos, const float scale);

	CPointLight(const Vector3f& pos, const Vector3f& color, const float scale);

	void applyLight(const int id);

};


#endif