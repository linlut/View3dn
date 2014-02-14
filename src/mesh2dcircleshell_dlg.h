#ifndef _INC_MESH2DCIR_SHELL_DLGxx11_H_
#define _INC_MESH2DCIR_SHELL_DLGxx11_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMesh2DCircleShellDlg: public CMeshPrimitiveDialog
{
private:	
	static double m_fR;
	static double m_fThickness;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;
	static int m_nArcDiv;
	static int m_nThkDiv;

public:

public:
    CMesh2DCircleShellDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMesh2DCircleShellDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif