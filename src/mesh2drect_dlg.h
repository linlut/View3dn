#ifndef _INC_MESH2D_RECT_DLG012_H_
#define _INC_MESH2D_RECT_DLG012_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMesh2DRectDlg: public CMeshPrimitiveDialog
{
private:
	static Vector3d m_vLowleft;
	static Vector3d m_vUpright;
	static Vector3d m_vCenter;
	static int m_nx;
	static int m_ny;

public:

public:
    CMesh2DRectDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMesh2DRectDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif