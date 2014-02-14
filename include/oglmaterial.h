//FILE: oglmaterial.h
#ifndef _INC_CIA3D_OGLMATERIAL_H_
#define _INC_CIA3D_OGLMATERIAL_H_

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <map>
#include <string>

//Check QT included or not
#ifndef USE_QT_LIB 
	class QImage
	{
	public:
		QImage(){}

		QImage (const char *){}

		int width(void) const{ return 0; }

		int height(void) const{ return 0; }

		unsigned char *bits(void) const
		{
			return 0;
		}

		int depth(void)
		{
			return 0;
		}
	};

	class QGLWidget
	{
	public:
		QGLWidget(){};
		static QImage convertToGLFormat(QImage &p)
		{
			static QImage q;
			return q;
		}
	};

#else
	//class QImage;    //QT's Image class
    #include <QImage.h>
    //#include <QtOpenGL.h>
    #include <qgl.h>
#endif

//Color texture or bump map texture
class OglTexture
{
public:
    std::string    texname;//name of the texture, we will use the file name as the texture name
    int textype;        //texture type: -1: undefined, 0: color, 1: bumpmap
    GLuint texid;        //OpenGL texture ID
    float texscale;        //scaling of the texture
    QImage *m_pImage;    //QT's image class

private:
    void zero(void)
    {
        texname="";
        textype = -1;
        texid = -1;
        texscale = 1;
        m_pImage = NULL;
    }

    void initForRendering(void)
    {
        QImage *image = m_pImage;
        if (m_pImage==NULL) return;
        const int w = image->width();
        const int h = image->height();
        unsigned char *prgb = image->bits();
        const int depth = image->depth();

        glGenTextures(1, &texid);                        // Create The Texture
        //Typical Texture Generation Using Data From The Bitmap
        glBindTexture(GL_TEXTURE_2D, texid);
        //glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, prgb);
        if (depth==32){
            glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, prgb);
        }
        else{
            printf("Not supported texture!\n");
            return;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);       
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    }

public:
    OglTexture(void)
    {
        zero();
    }

    ~OglTexture(void)
    {
        if (texid!=-1) glDeleteTextures(1, &texid);
        if (m_pImage) delete m_pImage;
    }

    OglTexture(const char *name, const int _texturetype=0, const float scale=1)
    {
        zero();
        texname = name;
        textype = _texturetype;
        texscale = scale;
    }

    bool loadImage(void)
    {
        if (m_pImage) delete m_pImage;
        m_pImage = new QImage(texname.c_str());
        if (m_pImage){
            const int w = m_pImage->width();
            const int h = m_pImage->height();
            if (w<=0 || h<=0){
                delete m_pImage;
                m_pImage = NULL;
            }
        }

        //Converts the image img into the unnamed format expected by OpenGL
        //functions such as glTexImage2D()
        if (m_pImage){
            QImage glimage = QGLWidget::convertToGLFormat(*m_pImage);
            *m_pImage = glimage;
        }

        return (m_pImage!=NULL);
    }

    void bind(void)
    {
        if (texid==-1)
            initForRendering();
        if (texid==-1) return;
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    void unbind(void)
    {
        if (texid==-1) return;
        glBindTexture(GL_TEXTURE_2D, 0);
    }

};


class OglTextureFactory
{
private:
    typedef std::map< std::string, OglTexture *> TextureFactory;
    TextureFactory texFactory;

    void _clear(void)
    {
        TextureFactory::iterator it=texFactory.begin();
        for (; it!= texFactory.end(); it++){
            if (it->second != NULL){
                delete it->second;
                it->second = NULL;
            }
        }
        texFactory.clear();
    }


public:
    OglTextureFactory(void): texFactory()
    {
    }

    ~OglTextureFactory(void)
    {
        _clear();
    }

    void clear(void)
    {
        _clear();
    }

    void clearFactory(void)
    {
        _clear();
    }

    void addEntry(const std::string& name, OglTexture *tex)
    {
        texFactory[name] = tex;
    }

    OglTexture* getEntry(const std::string& name)
    {
        OglTexture *tex = texFactory[name];
        return tex;
    }

    const char *getName(OglTexture* tex)
    {
        TextureFactory::iterator it=texFactory.begin();
        for (; it!= texFactory.end(); it++){
            if (it->second == tex){
                const char *p = it->first.c_str();
                return p;
            }
        }
        return NULL;
    }

    static OglTextureFactory *getInstance(void)
    {
        static OglTextureFactory fac;
        return &fac;
    }

    static void AddEntry(const std::string& name, OglTexture *tex)
    {
        getInstance()->addEntry(name, tex);
    }

    static OglTexture* GetEntry(const std::string& name)
    {
        return getInstance()->getEntry(name);
    }

    static const char *GetName(OglTexture* tex)
    {
        return getInstance()->GetName(tex);
    }

    static void ClearFactory(void)
    {
        getInstance()->clear();
    }
};


//OpenGL rendering pipeline material definition
class OglMaterial
{   
public:
    std::string    name;        /* name of material */

    GLfloat    ambient[4];        /* ambient component */
    GLfloat    diffuse[4];        /* diffuse component */
    GLfloat    specular[4];    /* specular component */
    GLfloat    emissive[4];    /* emmissive component */
    GLfloat    shininess;        /* specular exponent */

    bool useDiffuse;
    bool useSpecular;
    bool useAmbient;
    bool useEmissive;
    bool useShininess;
    bool activated;
    bool __xxx[2];            //padding

    //Color texture
    OglTexture *colortex;    //We use two textures for each mat: color texture
    OglTexture *bumptex;    // and bump mapping texture

    OglMaterial(void)
    {
        for (int i = 0; i < 3; i++){
            ambient[i] = (GLfloat)0.10;
            diffuse[i] = (GLfloat)0.75;
            specular[i] = (GLfloat)0.3;
            emissive[i] = (GLfloat)0.0;
        }
        ambient[3] = 1;
        diffuse[3] = 1;
        specular[3] = 1;
        emissive[3] = 1;
       
        shininess = 60;
        name = "DefaultOglMaterial";
       
        useAmbient = false;
        useDiffuse = true;
        useSpecular = false;
        useEmissive = false;
        useShininess = false;
        activated = false;

        colortex = NULL;
        bumptex = NULL;
    }

    OglMaterial(const OglMaterial& another)
    {
        *this = another;
        this->name = another.name;
    }

    ~OglMaterial()
    {}

    void setColor(const float r, const float g, const float b, const float a=1)
    {
        const float f[4] = { r, g, b, a };
        for (int i=0;i<4;i++){
            ambient[i] = f[i];
            diffuse[i] = f[i];
            specular[i] = f[i];
            emissive[i] = f[i];
        }
    }

    bool isTransparent(void)
    {
        OglMaterial &material = *this;
        return    (material.useDiffuse && material.diffuse[3] <= 0.999f);
    }

    void setupMaterial(const bool usetexture)
    {
        GLfloat blackcolor[4]={0, 0, 0, 1};
        GLfloat whitecolor[4]={1, 1, 1, 1};
        GLfloat negcolor[4]={0,0,0,1};
        OglMaterial &material = *this;

        if (material.isTransparent()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
        }

        glColor3f(1.0 , 1.0, 1.0);
        if (material.useAmbient)
            glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, material.ambient);
        else
            glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, blackcolor);

        if (material.useDiffuse){
            //Fix a minor problem in the Maya's OBJ file about Kd
            float matkd[4] = {material.diffuse[0], material.diffuse[1], material.diffuse[2], material.diffuse[3]};
            if (colortex){
                if (matkd[0]==0 && matkd[1]==0 && matkd[2]==0)
                    matkd[0]=1, matkd[1]=1, matkd[2]=1;
            }

            glMaterialfv (GL_FRONT, GL_DIFFUSE, matkd);
            negcolor[0]=1-matkd[0];
            negcolor[1]=1-matkd[1];
            negcolor[2]=1-matkd[2];
            glMaterialfv (GL_BACK, GL_DIFFUSE, negcolor);
        }
        else{
            glMaterialfv (GL_FRONT, GL_DIFFUSE, blackcolor);
            glMaterialfv (GL_BACK, GL_DIFFUSE, whitecolor);
        }

        if (material.useSpecular)
            glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, material.specular);
        else
            glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, blackcolor);

        if (material.useEmissive)
            glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, material.emissive);
        else
            glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, blackcolor);

        if (material.useShininess)
            glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);

        if (usetexture && colortex!=NULL)
            colortex->bind();
    }

    void unsetupMaterial(const bool usetexture)
    {
        if (usetexture && colortex!=NULL)
            colortex->unbind();
    }

    bool hasTexture(void)
    {
        return (colortex!=NULL) || (bumptex!=NULL);
    }

};

#endif
