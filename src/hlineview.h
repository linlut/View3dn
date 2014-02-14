/****************************************************************************


*****************************************************************************/

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include <qgl.h>
#include <qimage.h>
#include <triangle_mesh.h>
#include "cia3dfloor.h"
#include "scenegraph.h"
//#include "hline_removal.h"

class CTriangleObj;
class CQuadObj;
class CHiddenLineRemoval;
class ARFACE;

class CHlineViewer : public QGLViewer 
{

private:

	CGLDrawParms	*m_pDrawParms;			// drawing configuration

	CSceneGraph		*m_pSceneGraph;

	QString			m_strFileName;
	
	CHiddenLineRemoval *m_pHLR;

private:
	void convertTriangleObj2Arface(CTriangleObj *ptri, ARFACE** ppface);

	void convertQuadObj2Arface(CQuadObj *pquad, ARFACE** ppface);

	CHiddenLineRemoval* initHiddenlineClass(CGLDrawParms *pDrawParms, CSceneGraph *pSceneGraph);

	void setCamera2ScreenMatrix(double m[16], const int w, const int h);

public:

	//construction;
	CHlineViewer(CGLDrawParms *pDrawParms, CSceneGraph *pSceneGraph, double m[16], const int w, const int h, QWidget *parent,const char *name);

	~CHlineViewer(void);

	virtual void draw(void);


protected:

	virtual void keyPressEvent( QKeyEvent * e );


};

