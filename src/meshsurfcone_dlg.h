#ifndef _INC_MESHSURFCONE_DLG9904_H_
#define _INC_MESHSURFCONE_DLG9904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSurfConeDlg: public CMeshPrimitiveDialog
{
private:
	
	static int m_nArcDiv;
	static int m_nHgtDiv;
	static double m_fR;
	static double m_fH;
	static double m_fA1;
	static double m_fA2;
	static Vector3d m_vCenter;

public:

public:
    CMeshSurfConeDlg(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSurfConeDlg(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif