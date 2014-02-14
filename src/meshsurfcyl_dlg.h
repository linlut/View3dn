#ifndef _INC_MESHSURFCYL_DLG911_H_
#define _INC_MESHSURFCYL_DLG911_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSurfCylinderDlg: public CMeshPrimitiveDialog
{
private:	
	static double m_fR;
	static double m_fHeight;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;
	static int m_nArcDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshSurfCylinderDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSurfCylinderDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif