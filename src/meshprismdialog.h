#ifndef _INC_MESHPRISM_DIALOG_X9904_H_
#define _INC_MESHPRISM_DIALOG_X9904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshPrismDialog: public CMeshPrimitiveDialog
{
private:

	static Vector3d m_vVertex[6];	
	static Vector3d m_vTranslate;
	static double m_fH;

	static int m_nArcDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshPrismDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshPrismDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif