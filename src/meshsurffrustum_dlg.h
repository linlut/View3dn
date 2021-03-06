#ifndef _INC_MESHSURF_FRUSTUM_DLG94612341_H_
#define _INC_MESHSURF_FRUSTUM_DLG94612341_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSurfFrustumDlg: public CMeshPrimitiveDialog
{
private:	
	static double m_fR1;    ///bottom radius
	static double m_fR2;    ///top radius
	static double m_fHeight;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;
	static int m_nArcDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshSurfFrustumDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSurfFrustumDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif