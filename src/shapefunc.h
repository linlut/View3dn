//shapefunc.h

#ifndef __INC_SHAPE_FUNC_9036sghs_H_
#define __INC_SHAPE_FUNC_9036sghs_H_


#include <vector3d.h>
#include "fortranarray.h"


inline void 
xyzcart(const FREAL &u, const FREAL &v, const FREAL &r, M2 &xyzcur, const int pos, const int nums, M1& xyz0)
{
	//dimension xyzcur(3),xyz0(3),xyz01(3)
    const double pa180=3.1415926535897932384626433/180.0;
	FREAL uc, vc, xcar, ycar, zcar;
	switch(nums){
	case 2:		//cylindrical mapping
		uc=pa180*u;
		xcar=r*cos(uc)+xyz0(1);
		ycar=r*sin(uc)+xyz0(2);
		zcar=v+xyz0(3);
		break;
	case 3:		//spherical mapping
		uc=pa180*u;
		vc=pa180*v;
		xcar=r*cos(uc)*sin(vc)+xyz0(1);
		ycar=r*sin(uc)*sin(vc)+xyz0(2);
		zcar=r*cos(vc)+xyz0(3);
		break;
	default:
		assert(0);
		break;
	}
    xyzcur(1, pos)=xcar; 
	xyzcur(2, pos)=ycar; 
	xyzcur(3, pos)=zcar;
}


inline void 
xyzcart(const FREAL &u, const FREAL &v, const FREAL &r, const int mapping, Vector3d& vertex)
{
    const double pa180=PI/180.0;
	FREAL uc, vc, xcar, ycar, zcar;
	switch(mapping){
	case 2:		//cylindrical mapping
		uc=pa180*u;
		xcar=r*cos(uc);
		ycar=r*sin(uc);
		zcar=v;
		break;
	case 3:		//spherical mapping
		uc=pa180*u;
		vc=pa180*v;
		xcar=r*cos(uc)*sin(vc);
		ycar=r*sin(uc)*sin(vc);
		zcar=r*cos(vc);
		break;
	default:
		assert(0);
		break;
	}
    vertex.x=xcar; vertex.y=ycar; vertex.z=zcar;
}


inline void 
sphereMapping(const FREAL &u, const FREAL &v, const FREAL &r, Vector3d &vertex)
{
	xyzcart(u, v, r, 3, vertex);
}


void SHAPEGRD8_NODEDRI(FREAL SHAPE[9], const FREAL& EXISP, const FREAL& ETASP, const FREAL& ZTASP);


void mshKNodeZone(const int NNODE, MV3 &vertex,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1&iyend, IA1& izstr, IA1& izend,
		M3 &xyz, const int izn, IA1& itype);


void mesh4zn(MV3& vertex, IA1& nzsti,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1& iyend, IA1& izstr, IA1& izend,
		M3& xyz, const int izn, IA1& itype);


inline void mesh8zn(MV3& vertex,
		IA1& ixstr, IA1& ixend, IA1& iystr, IA1& iyend, IA1& izstr, IA1& izend,
		M3& xyz, const int izn, IA1& itype)
{
    mshKNodeZone(8, vertex, ixstr, ixend, iystr, iyend, izstr, izend, xyz, izn, itype);
}


inline void msh20zn(MV3 &vertex,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1&iyend, IA1& izstr, IA1& izend,
		M3 &xyz, const int izn, IA1& itype)
{
    mshKNodeZone(20, vertex, ixstr, ixend, iystr, iyend, izstr, izend, xyz, izn, itype);
}


inline void msh21zn(MV3 &vertex,
		IA1 &ixstr, IA1& ixend, IA1& iystr, IA1&iyend, IA1& izstr, IA1& izend,
		M3 &xyz, const int izn, IA1& itype)
{
    mshKNodeZone(21, vertex, ixstr, ixend, iystr, iyend, izstr, izend, xyz, izn, itype);
}


#endif
