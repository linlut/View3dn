#ifndef _INC_MESH2d_TRIANGLE_DIALOG_X9904_H_
#define _INC_MESH2d_TRIANGLE_DIALOG_X9904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMesh2DTriangleDlg: public CMeshPrimitiveDialog
{
private:

	static Vector3d m_vVertex1;	
	static Vector3d m_vVertex2;	
	static Vector3d m_vVertex3;	
	static Vector3d m_vTranslate;
	static int m_nArcDiv;

public:

public:
    CMesh2DTriangleDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMesh2DTriangleDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif