#ifndef _INC_MESH_SURFBOX_DLG111_H_
#define _INC_MESH_SURFBOX_DLG111_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSurfBoxDlg: public CMeshPrimitiveDialog
{
private:	
	static Vector3d m_vLowleft;
	static Vector3d m_vUpright;
	static Vector3d m_vCenter;
	static int m_nx;
	static int m_ny;
	static int m_nz;

public:

public:
    CMeshSurfBoxDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSurfBoxDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif