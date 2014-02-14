/****************************************************************************

 This file is part of the QGLViewer library.
 Copyright (C) 2002, 2003, 2004, 2005 Gilles Debunne (Gilles.Debunne@imag.fr)
 Version 1.3.9-4, released on February 16, 2005.

 http://artis.imag.fr/Members/Gilles.Debunne/QGLViewer

 libQGLViewer is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 libQGLViewer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with libQGLViewer; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#include "glwin.h"
#include <qapplication.h>
#include <time.h>


using namespace std;
using namespace qglviewer;


CGLWin* pmainwindow;

void Draw3DText(double x, double y, double z, char *txt)
{
	//QGLWidget *pwin = pmainwindow->m_pGLUIWin;
	//pwin->renderText(x, y, z, QString(txt));
}

/*
void WorldPoint2Screen(double wp[3], double sp[3])
{
	float src[3], res[3];

	src[0]=wp[0], src[1]=wp[1], src[2]=wp[2];
	Camera* pcamera = pmainwindow->m_pGLUIWin->camera();
	//pcamera->getProjectedCoordinatesOf(src, res);
	pcamera->projectedCoordinatesOf(src, res);
	sp[0]=res[0], sp[1]=res[1], sp[2]=res[2];
}
*/
void WorldPoint2Screen(double wp[3], double sp[3])
{
	qglviewer::Vec src(wp[0], wp[1], wp[2]);
	qglviewer::Vec res;

	Camera* pcamera = pmainwindow->m_pGLUIWin->camera();
	res = pcamera->projectedCoordinatesOf(src);
	sp[0]=res.x, sp[1]=res.y, sp[2]=res.z;
}

void WorldPoint2Camera(double wp[3], double cp[3])
{
	float src[3], res[3];

	src[0]=wp[0], src[1]=wp[1], src[2]=wp[2];
	Camera* pcamera = pmainwindow->m_pGLUIWin->camera();
	pcamera->getCameraCoordinatesOf(src, res);
	cp[0]=res[0], cp[1]=res[1], cp[2]=res[2];
}


void ScreenPoint2World(double sp[3], double wp[3])
{
	float src[3], res[3];

	src[0]=sp[0], src[1]=sp[1], src[2]=sp[2];
	Camera* pcamera = pmainwindow->m_pGLUIWin->camera();
	pcamera->getProjectedCoordinatesOf(src, res);
	wp[0]=res[0], wp[1]=res[1], wp[2]=res[2];
}

