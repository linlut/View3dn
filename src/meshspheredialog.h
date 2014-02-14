#ifndef _INC_MESHSPHERE_DIALOG3979904_H_
#define _INC_MESHSPHERE_DIALOG3979904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSphereDialog: public CMeshPrimitiveDialog
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
    CMeshSphereDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSphereDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif