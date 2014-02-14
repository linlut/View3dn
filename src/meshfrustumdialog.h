#ifndef _INC_MESHFRUSTUM_DIALOG94_H_
#define _INC_MESHFRUSTUM_DIALOG94_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshFrustumDialog: public CMeshPrimitiveDialog
{
private:
	
	static double m_fR1;
	static double m_fR2;
	static double m_fA1;
	static double m_fA2;
	static double m_fH;
	static Vector3d m_vCenter;
	static int m_nDivArc;
	static int m_nDivHgt;

public:

public:
    CMeshFrustumDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshFrustumDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif