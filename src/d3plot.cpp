//file: d3plot.cpp
//purpose: handle d3_plot format by LS_DYNA

#include <vector>
#include "cia3dobj.h"
#include "tmeshobj.h"
#include "qmeshobj.h"
#include "pointobj.h"
#include "tetraobj.h"
#include "hexobj.h"
#include "lineobj.h"
#include "dynamic_array.h"
#include "volumemeshio.h"

using namespace std;

static CPolyObj* loadD3PlotFile(const char *fname)
{
	/*
	CCia3dObj obj;
	if (obj.LoadFile(fname, 1.0)){
		CPolyObj *p = dynamic_cast<CPolyObj*>(obj.m_pObjList[0]);
		if (p != NULL)
			obj.m_pObjList[0] = NULL;
		return p;
	}
	*/
	return NULL;
}

static bool _loadD3Plot = CPolyObjLoaderFactory::AddEntry("", loadD3PlotFile);



class D3PlotMemoryMap
{
private:
	vector<Vector3d> m_vertices;

	vector<Vector2i> m_lines;

	vector<Vector4i> m_quads;

	vector<Vector8i> m_cubes;

};
