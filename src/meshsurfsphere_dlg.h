#ifndef _INC_MESHSURFSPHERE_DIALOG14_H_
#define _INC_MESHSURFSPHERE_DIALOG14_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSurfSphereDlg: public CMeshPrimitiveDialog
{
private:
	
	static int m_nArcDiv;
	static double m_fR;
	static double m_fA1;
	static double m_fA2;
	static double m_fB1;
	static double m_fB2;
	static Vector3d m_vCenter;

public:

public:
    CMeshSurfSphereDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSurfSphereDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif