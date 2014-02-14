#ifndef _INC_MESHCYL_DIALOG9904_H_
#define _INC_MESHCYL_DIALOG9904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshCylinderDialog: public CMeshPrimitiveDialog
{
private:
	
	static double m_fR;
	static double m_fA1;
	static double m_fA2;
	static double m_fH;
	static Vector3d m_vCenter;
	static int m_nDivArc;
	static int m_nDivHgt;

public:

public:
    CMeshCylinderDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshCylinderDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);

    virtual void resize(int w, int h);
};

#endif