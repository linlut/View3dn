//FILE: ZQGLViewer.h

#ifndef _CREST_GUI_ZQGLVIEWER_H
#define _CREST_GUI_ZQGLVIEWER_H

#include <qgl.h>
#include <qtimer.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <QGLViewer/qglviewer.h>


class ZQGLViewer :public QGLViewer
{
private:
	GLuint m_fboMRT[6];

private:
	void _reshape(const int w, const int h);

public:
 	ZQGLViewer(QWidget *parent=0, const QGLWidget *shareWidget=0, Qt::WFlags flags=0);

	virtual ~ZQGLViewer(void);

	GLuint *getFrameBufferObjectPointer(void)
	{
		return m_fboMRT;
	}

protected:
	virtual void init(void);
	virtual void resizeGL(int width, int height);
};

#endif


