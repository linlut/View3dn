/****************************************************************************
FILE: view3d.h

*****************************************************************************/
#ifndef __INC_QTVIEW3D_H__
#define __INC_QTVIEW3D_H__


#include <qmutex.h>
#include <qgl.h>
#include <qimage.h>
#include <qtimer.h>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>

#include <triangle_mesh.h>
#include "cia3dfloor.h"
#include "scenegraph.h"


using namespace std;
using namespace qglviewer;


//class QMutex;
class CGLWin;


class Viewer : public QGLViewer 
{
	 Q_OBJECT

public:
	CFloor      m_Floor;
	CSceneGraph m_SceneGraph;
	QImage		*m_pBGImage;
    
	//construction;
	Viewer(const QGLFormat &format, QWidget *parent,const char *name);

	virtual ~Viewer(void)
	{
	}

	void setDrawingData(CGLDrawParms *pDrawStyle);

    void drawTextGlut(int x, int y, const char *text);

    void drawOsd(const float fps);

	void setMirrorDrawingData(CGLDrawParms *pDrawStyle)
	{
		//_pDrawMirrorParms = pDrawStyle;
	}

	void setRotationDrawingData(CGLDrawParms *pDrawStyle, const int rotationid)
	{
		assert(rotationid>=0 && rotationid<3);
		//_pRotationDrawParms[rotationid] = pDrawStyle;
	}

	CGLDrawParms* getRotationDrawingData(const int rotationid)
	{
		assert(rotationid>=0 && rotationid<3);
		//return _pRotationDrawParms[rotationid];
	}

	void setAnimationParameters(char *fin, char *fout, const int startcount, const int endcount, const int inc) 
	{
		strcpy(_fname_animinput_common, fin);
		strcpy(_fname_animoutput_common, fout);

		_nStartCount = startcount;
		_nEndCount = endcount;
		_nCurrentCount = startcount;
		_nIncCount = inc;
	}

	//change eye distance in stereo mode
	void incEyeDistance(const int sign);

	//render an animation;
	void doAnimation(void);

	//draw the legend for opengl texturing
	void DrawLegend(void);

	void draw(void)  //reload a virtual function
	{
		__glDraw();
	}

    void toggleUseVbo()
    {
        m_useVbo = !m_useVbo;
    }

    void toggleUseDispList()
    {
        m_useDispList = !m_useDispList;
    }

public slots:
	void activeSceneNodeChanged(CSceneNode* p)
	{
		if (p==NULL)
			this->setManipulatedFrame(NULL);
		else
			this->setManipulatedFrame(p->m_pFrame);
	}

	void pickingCursorChanged(QCursor *qcur)
	{
		this->setCursor(*qcur);
	}

	void updateGLWindowsBoundingBox(const Vector3d& b1, const Vector3d& b2)
	{
		Vec a1(b1.x, b1.y, b1.z);
		Vec a2(b2.x, b2.y, b2.z);
		this->setSceneBoundingBox(a1, a2);
	}

	void onDraw(void);

protected:
    QTimer m_timer;

	bool m_bFastDraw;

	bool m_bRepeatPlay;

	int m_nTimeDelay;

	void resetCurrentAnimationCounter(void);

	void invalidCurrentAnimationCounter(void);

	virtual void init(void);

	virtual void fastDraw(void);

	virtual void keyPressEvent( QKeyEvent * e );

	//for picking an object
	virtual void drawWithNames(void);

	virtual void postSelection(int x, int y);

	virtual void postSelection(const QPoint &point)
	{
		postSelection(point.x(), point.y());
	}

	virtual QString helpString() const;

	friend class CGLWin;

private:
	QMutex m_animMutex;
	CGLDrawParms  *_pDrawParms;				// drawing configuration

	//animation rendering;
	char _fname_animinput_common[200];
	char _fname_animoutput_common[200];
	int _nStartCount;
	int _nEndCount;
	int _nIncCount;
	int _nCurrentCount;
    bool m_useVbo;
    bool m_useDispList;

	void glDrawObject(void);

	bool isInAnimation(void)
	{
		if (_nCurrentCount<=_nEndCount)
			return true;
		return false;
	}

	void increaseAnimationCounter(void)
	{
		m_animMutex.lock();
		_nCurrentCount += _nIncCount;
		m_animMutex.unlock();
	}

	int get_counter(void)
	{
		return _nCurrentCount;
	}

	void getInputFileNames(char *fname)
	{
		//sprintf(fname, "%s%d.plt", _fname_animinput_common, _nCurrentCount); 
		sprintf(fname, _fname_animinput_common, _nCurrentCount); 
	}

	void getOutputAnimationFileNames(char *fname)
	{
		if (_fname_animoutput_common[0]==0)
			fname[0]=0;
		else
			sprintf(fname, "%s", _fname_animoutput_common); 
	}

	void _showBackgroundImage(void);

	void __glDraw(void);

	void _drawHorizonTextureBlocks(CGLDrawParms* p, const int sw, const int sh, const int pos, unsigned char  rgba[256][4], const int n);

	void _drawVerticalTextureBlocks(CGLDrawParms* p, const int sw, const int sh, const int pos, unsigned char  rgba[256][4], const int n);

	void _drawTextureBlocks(CGLDrawParms* p, const int pos, const int orientation, unsigned char  rgba[256][4], const int n);

	void _doAnimationLoop(const char *foutname, const char *curext, void *p);


signals:
	void printMessage(const QString &);

};

#endif