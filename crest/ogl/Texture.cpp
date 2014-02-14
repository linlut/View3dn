//FILE: texture.cpp

#include <assert.h>
#include <qimage.h>
#include <iostream>
#include <crest/ogl/texture.h>

namespace cia3d{
namespace ogl{

void Texture::init(void)
{
	const int w = image->width();
	const int h = image->height();
	unsigned char *prgb = image->bits();
	const int depth = image->depth();

	glGenTextures(1, &id);						// Create The Texture
	std::cout << "Create Texture"<<std::endl;
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, id);
//	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, prgb);
	if (depth==32){
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, prgb);
	}
	else{
		assert(0);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::bind(void)
{
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind(void)
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

QImage* Texture::getImage(void)
{
	return image;
}
Texture::~Texture(void)
{
	glDeleteTextures(1, &id);
	delete image;
}

} // namespace ogl
} // namespace cia3d