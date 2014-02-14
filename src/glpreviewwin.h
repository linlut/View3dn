//File: glpreviewwin.h

#ifndef __INC_GLPREVIEWWIN312_H__
#define __INC_GLPREVIEWWIN312_H__


#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include <qgl.h>
#include <qtimer.h>

#include "vector3f.h"
#include "drawparms.h"
#include "polyobj.h"
#include "nametableitem.h"


using namespace std;
using namespace qglviewer;


class CGLPreviewWin : public QGLViewer 
{

private:

	CGLDrawParms m_DrawParms;		//drawing configuration

	CNameTableItem *m_pNameTable;

	int m_nTableLen;

	int m_nTableType;

	CPolyObj * m_pObj;

	QTimer *m_timerStep;

	int m_inRenderingFunc;

public:
    
    void releaseObject(void)
    {
        if (m_pObj) delete m_pObj;
        m_pObj = NULL;
    }

    CPolyObj * acquireObject(void)
    {
        CPolyObj * tmp = m_pObj;
        m_pObj=NULL;
        return tmp;
    }

    void dumpObjectPointer(void)
    {
        printf("Object pointer is %p\n", m_pObj); 
    }

private:

	void _initWindow(void);

	void _draw(void);

	//this func is called by the draw() func
	void _drawNameTable(void);

public:

	CGLPreviewWin(QWidget *parent, const char *windowname, const QGLWidget* shareWidget=NULL);

	virtual ~CGLPreviewWin();

	void setObject(CPolyObj *pobj);

	void setNameTable(CNameTableItem *pNameTable, const int nTableLen, const int nTableType)
	{
		m_pNameTable = pNameTable;
		m_nTableLen = nTableLen;
		m_nTableType = nTableType;
	}

	
protected:
	//this func is called before the first OpenGL draw
	virtual void init(void);

	//Opengl draw func
	virtual void draw(void)
	{
		if (m_inRenderingFunc) return;
		m_inRenderingFunc = 1;
		_draw();
		_drawNameTable();
		m_inRenderingFunc = 0;
	}
	
	//Opengl fast draw
	virtual void fastDraw(void)
	{
		_draw();
		_drawNameTable();
	}

	//keyboard event processing
	virtual void keyPressEvent( QKeyEvent * e );

};




#endif