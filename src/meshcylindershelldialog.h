#ifndef _INC_MESHCYLINDERSHELL_DLGX1_H_
#define _INC_MESHCYLINDERSHELL_DLGX1_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshCylinderShellDialog: public CMeshPrimitiveDialog
{
private:	
	static double m_fR;
	static double m_fHeight;
	static double m_fThickness;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;
	static int m_nArcDiv;
	static int m_nThkDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshCylinderShellDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshCylinderShellDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif