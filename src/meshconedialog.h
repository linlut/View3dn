#ifndef _INC_MESHCONE_DIALOG39904_H_
#define _INC_MESHCONE_DIALOG39904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshConeDialog: public CMeshPrimitiveDialog
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
    CMeshConeDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshConeDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif