

#ifndef _INC_MESH_2DCIRCLESHELL11_H_
#define _INC_MESH_2DCIRCLESHELL11_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMesh2DCircleDlg: public CMeshPrimitiveDialog
{
private:
	
	static double m_fR;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;
	static int m_nDivArc;

public:

public:
    CMesh2DCircleDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMesh2DCircleDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);

    virtual void resize(int w, int h);
};

#endif