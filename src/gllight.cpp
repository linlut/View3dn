#include "gllight.h"

void CPointLight::init(const Vector3f& pos, const Vector3f& color, const float scale)
{
	const float ka = 0.0500f;
	const float kd = 1.0f;
	const float ks = 1.0f;
	m_bEnable = true;
	m_position = pos;
	m_ambient = color * scale * ka;
	m_diffuse = color * scale;
	m_specular = color * scale;
}


CPointLight::CPointLight(void)
{
	const Vector3f pos(1,1,1);
	const Vector3f color(1,1,1);
	const float scale=1;
	init(pos, color, scale);
}


CPointLight::CPointLight(const Vector3f& pos)
{
	const Vector3f color(1,1,1);
	const float scale=1;
	init(pos, color, scale);
}


CPointLight::CPointLight(const Vector3f& pos, const float scale)
{
	const Vector3f color(1,1,1);
	init(pos, color, scale);
}


CPointLight::CPointLight(const Vector3f& pos, const Vector3f& color, const float scale)
{
	init(pos, color, scale);
}


void CPointLight::applyLight(const int id)
{
	GLenum glLightID = GL_LIGHT0 + id;
	
	glEnable(glLightID);
	glLightfv(glLightID, GL_POSITION, &m_position.x);
	glLightfv(glLightID, GL_AMBIENT, &m_ambient.x);
	glLightfv(glLightID, GL_DIFFUSE, &m_diffuse.x);
	glLightfv(glLightID, GL_SPECULAR, &m_specular.x);
}