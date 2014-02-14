#ifndef _INC_MESHSPHERESHELL_DLG911_H_
#define _INC_MESHSPHERESHELL_DLG911_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSphereShellDialog: public CMeshPrimitiveDialog
{
private:	
	static double m_fR;
	static double m_fH;
	static double m_fA1;
	static double m_fA2;
	static double m_fB1;
	static double m_fB2;
	static Vector3d m_vCenter;
	static int m_nArcDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshSphereShellDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSphereShellDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif