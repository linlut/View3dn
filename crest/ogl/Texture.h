//File: texture.h

#ifndef CIA3D_GL_TEXTURE_H
#define CIA3D_GL_TEXTURE_H

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
#include <GL/gl.h>

class QImage;

namespace cia3d{
namespace ogl{

class Texture
{

private:
	QImage *image;
	GLuint id;

public:
	Texture(QImage *img)
	{
		image = img;
		id = 0;
	}

	~Texture();

	QImage* getImage(void);

	void   bind(void);
	void   unbind(void);
	void   init (void);
};


} // namespace gl
} // namespace cia3d

#endif
