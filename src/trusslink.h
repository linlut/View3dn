//FILE: trusslink.h
#ifndef _INC_TRUSSLINK_H_
#define _INC_TRUSSLINK_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <vector>
#include <sysconf.h>
#include <GL/gl.h>
#include <polyobj.h>
#include <vectorall.h>

class CPolyObj;


class CTrussEdgeAux{
public:
	int ix;					//Node A index 
	int iy;					//Node B index
	float mass1;			//Mass of the two nodes of truss
	float mass2;			//
	float kk1;				//kk1 = E(m1+m2)/(rho*L0^2)
	float kk2;				//kk2 = 2*L0^3	
	double tvol;			//truss volume on this edge, which = tmass/RHO
	double tlen;			//currnet truss length 
	double len0;			//initial truss length 
	double len0_inv;		//1.0/initial truss length 
	unsigned char matid;	//material ID
	unsigned char geo;		//geo flag
	char tmp[2];			//padding

public:
	CTrussEdgeAux(){};
	~CTrussEdgeAux(){};
};

typedef CTrussEdgeAux CTruss;

//Build truss structure for a poly? into the truss set
extern void buildTruss(CPolyObj * pobj, const int adapt_type, std::vector<CTruss>& truss);

//Export truss edges to a povray file
extern void exportTrussPov(FILE *fp, Vector3d *pVertex, const int nv, const double *matrix, std::vector<CTruss>& truss);


#endif