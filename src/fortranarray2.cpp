//fortranarray.cpp

#include "fortranarray.h"
#include "shapefunc.h"


#define call 
const FREAL FQUARTER=0.25;
const FREAL FHALF=0.5;

extern void _setSphControlVertices(M3& xyz, M2& xyzo, M2& xyz1, M2& xyz2);



//=========================CONE=======================================

inline void 
_coneMapping(const FREAL &u, const FREAL &v, const FREAL &R, const FREAL &H, Vector3d &vertex)
{
	const FREAL uc=u;
	const FREAL r = v*R/H;
    vertex.x=r*cos(uc); 
	vertex.y=r*sin(uc);
	vertex.z=H-v;
}


inline void 
coneMapping(const FREAL &u, const FREAL &v, const FREAL &R, const FREAL &H, Vector3d &vertex)
{
    const double pa180=3.1415926535897932384626433/180.0;
	const FREAL uc=pa180*u;
    _coneMapping(uc, v, R, H, vertex);
}


inline void 
coneMapping(const FREAL &u, const FREAL &v, const FREAL &R, const FREAL &H, M2 &xyzcur, const int pos)
{
    Vector3d vertex;
    coneMapping(u, v, R, H, vertex);
    xyzcur(1, pos)=vertex.x; 
	xyzcur(2, pos)=vertex.y;
	xyzcur(3, pos)=vertex.z;
}


inline void projectPointOnCone(const Vector3d & pt, const FREAL R, const FREAL H, Vector3d & pt2)
{
    FREAL v = H - pt.z;
    FREAL u = atan2(pt.y, pt.x);
    _coneMapping(u, v, R, H, pt2);
}


static void 
_cone(M3 &xyz, const int izn, M1& alpha, const FREAL& r, const FREAL& h, M2& xyzo, M2& xyz1, M2& xyz2)
{
	int l, k;
	M1 beta(2); 
//c	node 1
	for (k=1; k<=3; k++) xyzo(k,1)=0;
//c	node 2
	beta(1)=0, beta(2)=h;
    FREAL u=alpha(1);
    FREAL v=beta(2);
    coneMapping(u,v,r,h,xyzo,2);
//c	node 3
    u=alpha(2);
    v=beta(2);
    coneMapping(u,v,r,h,xyzo,3);
//c	node 4
    u=alpha(2);
    v=beta(1);
    coneMapping(u,v,r,h,xyzo,4);
    for (k=1; k<=3; k++)
		for (l=1; l<=4; l++)
			xyz(k,l,izn)=xyzo(k,l);
//c==   generate midpoint coordinates which are nodal coordinates for each sub-zone
//c     midpoint of surface 1-2-3 (c1)
	const FREAL alpha12 = (alpha(2)+alpha(1))*FHALF;
	const FREAL beta12 = (beta(2)+beta(1))*FHALF;
    u=alpha12;
    v=beta(2);
    FREAL rh=r*0.62;
    coneMapping(u,v,r,h,xyz1,1);
//c     midpoint of side 1-2 (c2)
    u=alpha(1);
    v=beta(2);
    rh=r/2;
    coneMapping(u,v,rh,h,xyz1,2);
//c     midpoint of side 2-3 (c3)
    u=alpha12;
    v=beta(2);
    rh=r;
    coneMapping(u,v,r,h,xyz1,3);
//c     midpoint of side 3-1 (c4)
    u=alpha(2);
    v=beta(2);
    rh=r/2;
    coneMapping(u,v,r,h,xyz1,4);
//c     midpoint of body 1-2-3-4 (c1')
    u=alpha12;
    v=beta12; 
    rh=r*0.62;
    coneMapping(u,v,r,h,xyz2,1);
//c     midpoint of surface 1-2-4 (c2')
    u=alpha(1);
    v=beta12;
    rh=r*0.62;
    coneMapping(u,v,r,h,xyz2,2);
//c     midpoint of surface 2-3-4 (c3')
    u=alpha12;
    v=0.62*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz2,3);
//c     midpoint of surface 1-3-4 (c4')
    u=alpha(2);
    v=beta12;
    rh=r*0.62;
    coneMapping(u,v,r,h,xyz2,4);
//c     midpoint of side 2-4 (c5')
    u=alpha(1);
    v=(beta(1)+beta(2))*FHALF;
    rh=r;
    coneMapping(u,v,r,h,xyz2,5);
//c     midpoint of side 3-4 (c6')
    u=alpha(2);
    v=(beta(1)+beta(2))*FHALF;
    rh=r;
    coneMapping(u,v,r,h,xyz2,6);
//c     midpoint of side 1-4 (c7')
    u=alpha(1);
    v=beta(1);
    rh=r/2;
    coneMapping(u,v,r,h,xyz2,7);
//c==   generate midpoint coordinates for sub-zones sub-zone 2
    for (k=1; k<=3; k++){
		xyz1(k,5)=(xyz1(k,2)+xyzo(k,2))*FHALF; 
		xyz1(k,7)=(xyz1(k,1)+xyz1(k,3))*FHALF; 
		xyz1(k,8)=(xyz1(k,1)+xyz1(k,2))*FHALF; 
		xyz1(k,9)=(xyz1(k,2)+xyz2(k,2))*FHALF; 
		xyz1(k,12)=(xyz1(k,1)+xyz2(k,1))*FHALF; 
		xyz1(k,13)=(xyz2(k,2)+xyz2(k,5))*FHALF; 
		xyz1(k,15)=(xyz2(k,1)+xyz2(k,3))*FHALF;
		xyz1(k,16)=(xyz2(k,1)+xyz2(k,2))*FHALF;
	}
//c     midpoint of side 2-c3
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=beta(2);
    rh=r;
    coneMapping(u,v,r,h,xyz1,6);
//c     midpoint of side 2-c5'
    u=alpha(1);
    v=0.75*(beta(2)-beta(1))+beta(1);
    rh=r;
    coneMapping(u,v,r,h,xyz1,10);
//c     midpoint of side c3-c3'
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0.81*(beta(2)-beta(1))+beta(1);
    rh=r;
    coneMapping(u,v,r,h,xyz1,11);
//c     midpoint of side c5'-c3'
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    rh=r;
    coneMapping(u,v,r,h,xyz1,14);

//c*    sub-zone 3
    for (k=1; k<=3; k++){
		xyz1(k,17)=(xyz1(k,1)+xyz1(k,3))*FHALF;
		xyz1(k,19)=(xyzo(k,3)+xyz1(k,4))*FHALF; 
		xyz1(k,20)=(xyz1(k,4)+xyz1(k,1))*FHALF; 
		xyz1(k,21)=(xyz1(k,1)+xyz2(k,1))*FHALF; 
		xyz1(k,24)=(xyz1(k,4)+xyz2(k,4))*FHALF; 
		xyz1(k,25)=(xyz2(k,1)+xyz2(k,3))*FHALF; 
		xyz1(k,27)=(xyz2(k,4)+xyz2(k,6))*FHALF; 
		xyz1(k,28)=(xyz2(k,4)+xyz2(k,1))*FHALF; 
	}
//c     midpoint of side c3-3
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=beta(2);
    rh=r;
    coneMapping(u,v,r,h,xyz1,18);
//c     midpoint of side c3-c3'
	u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0.81*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz1,22);
//c     midpoint of side 3-c6'
    u=alpha(2);
    v=0.75*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz1,23);
//c     midpoint of side c3'-c6'
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz1,26);
//c*    sub-zone 4
    for (k=1; k<=3; k++){
		xyz2(k,8)=(xyz2(k,2)+xyz2(k,7))*FHALF; 
		xyz2(k,9)=(xyz2(k,1)+xyz2(k,2))*FHALF; 
		xyz2(k,10)=(xyz2(k,1)+xyz2(k,4))*FHALF; 
		xyz2(k,11)=(xyz2(k,4)+xyz2(k,7))*FHALF; 
		xyz2(k,12)=(xyz2(k,7)+xyzo(k,4))*FHALF; 
		xyz2(k,13)=(xyz2(k,2)+xyz2(k,5))*FHALF; 
		xyz2(k,14)=(xyz2(k,3)+xyz2(k,1))*FHALF; 
		xyz2(k,15)=(xyz2(k,6)+xyz2(k,4))*FHALF;
	}
//c     midpoint of side c5'-c3'
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz2,17);
//c     midpoint of side c5'-4
    u=alpha(1);
    v=0.25*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz2,16);
//c     midpoint of side c3'-c6'
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz2,18);
//c     midpoint of side 4-c6'
    u=alpha(2);
    v=0.25*(beta(2)-beta(1))+beta(1);
    coneMapping(u,v,r,h,xyz2,19);
}


static inline void 
_computeAvgWeight(const FREAL r, const FREAL h, FREAL &th, FREAL& to, FREAL& tr)
{
    FREAL wh=(h/(h+r));
    FREAL wr=1.0-wh;
    FREAL wo=_MIN_(wr, wh);
    FREAL sum=wh+wr+wo;
    const FREAL QT=0.25;
    th=wh/sum*QT+QT;
    to=wo/sum*QT+QT;
    tr=wr/sum*QT+QT;
}


static void 
setControlVertices(const FREAL& a1, const FREAL& a2, const FREAL r, const FREAL h, M3 &xyz)
{
	const FREAL K1=0.62;
	Vector3d v1, v2, v3, v4;	//the 4 control vertices
	const FREAL b1=0;
	const FREAL b2=90;
	const FREAL r2=r*0.5;
	const FREAL a12 = (a1+a2)*0.5;
	const int izone1=1;
	const int izone2=2;
	const int izone3=3;
	const int izone4=4;
	int i;

	//init for vertices v1--v4;
	v1=Vector3d(0,0,0);
	sphereMapping(a1, b2, r, v2);
	sphereMapping(a2, b2, r, v3);
	v4.x=v4.y=0, v4.z=h;

	//set first zone
	const Vector3d v12=(v1+v2)*FHALF;
	const Vector3d v13=(v1+v3)*0.5;
	const Vector3d v14=(v1+v4)*0.5; 
	Vector3d v23, v24, v34;
	sphereMapping(a12, b2, r, v23);
	v24=(v2+v4)*0.5;
	v34=(v3+v4)*0.5;

    FREAL TH, TR, TO;
    _computeAvgWeight(r, h, TH, TO, TR);
	Vector3d v123, v124, v134, v234;
	sphereMapping(a12, b2, r*K1, v123);
	v124=v1*TO+v2*TR+v4*TH;
	v134=v1*TO+v3*TR+v4*TH;
	const FREAL tt=0.58;
	v234=v23*tt+v4*(1-tt);
	const Vector3d v1234=(v1+v2+v3+v4)*0.25;

	//=====zone1============================
	Vector3dToFMatrix(v1, xyz, 1, izone1);
	Vector3dToFMatrix(v12, xyz, 2, izone1);
	Vector3dToFMatrix(v123, xyz, 3, izone1);
	Vector3dToFMatrix(v13, xyz, 4, izone1);
	Vector3dToFMatrix(v14, xyz, 5, izone1);
	Vector3dToFMatrix(v124, xyz, 6, izone1);
	Vector3dToFMatrix(v1234, xyz, 7, izone1);
	Vector3dToFMatrix(v134, xyz, 8, izone1);

	//=====zone2============================
	Vector3dToFMatrix(v12, xyz, 1, izone2);
	Vector3dToFMatrix(v2, xyz, 2, izone2);
	Vector3dToFMatrix(v23, xyz, 3, izone2);
	Vector3dToFMatrix(v123, xyz, 4, izone2);
	Vector3dToFMatrix(v124, xyz, 5, izone2);
	Vector3dToFMatrix(v24, xyz, 6, izone2);
	Vector3dToFMatrix(v234, xyz, 7, izone2);
	Vector3dToFMatrix(v1234, xyz, 8, izone2);

	Vector3d v2_23, v24_234, v1234_234, v1234_124;
	Vector3d v12_124, v2_24, v23_234, v123_1234;
	sphereMapping((a1+a12)*0.5, b2, r, v2_23);
	coneMapping((a1+a12)*0.5, h*0.5, r, h, v24_234);
	v1234_234=(v1234+v234)*0.5;
	v1234_124=(v1234+v124)*0.5;
	v12_124=(v12+v124)*0.5;
	v2_24=(v2+v24)*0.5;
	v23_234=(v23+v234)*0.5;
	v123_1234=(v123+v1234)*0.5;

	Vector3dToFMatrix((v12+v2)*0.5, xyz, 9, izone2);
	Vector3dToFMatrix(v2_23, xyz, 10, izone2);
	Vector3dToFMatrix((v23+v123)*0.5, xyz, 11, izone2);
	Vector3dToFMatrix((v12+v123)*0.5, xyz, 12, izone2);

	i=13;
    Vector3dToFMatrix(v12_124, xyz, i++, izone2);
	Vector3dToFMatrix(v2_24, xyz, i++, izone2);
	Vector3dToFMatrix(v23_234, xyz, i++, izone2);
	Vector3dToFMatrix(v123_1234, xyz, i++, izone2);	

	Vector3dToFMatrix((v124+v24)*0.5, xyz, i++, izone2);
	Vector3dToFMatrix(v24_234, xyz, i++, izone2);
	Vector3dToFMatrix(v1234_234, xyz, i++, izone2);
	Vector3dToFMatrix(v1234_124, xyz, i++, izone2);

	//=====zone3============================
	Vector3dToFMatrix(v123, xyz, 1, izone3);
	Vector3dToFMatrix(v23, xyz, 2, izone3);
	Vector3dToFMatrix(v3, xyz, 3, izone3);
	Vector3dToFMatrix(v13, xyz, 4, izone3);
	Vector3dToFMatrix(v1234, xyz, 5, izone3);
	Vector3dToFMatrix(v234, xyz, 6, izone3);
	Vector3dToFMatrix(v34, xyz, 7, izone3);
	Vector3dToFMatrix(v134, xyz, 8, izone3);

	Vector3d v23_3, v234_34;
	sphereMapping((a12+a2)*0.5, b2, r, v23_3);
	coneMapping((a2+a12)*0.5, h*0.5, r, h, v234_34);

	Vector3dToFMatrix((v123+v23)*0.5, xyz, 9, izone3);
	Vector3dToFMatrix(v23_3, xyz, 10, izone3);
	Vector3dToFMatrix((v13+v3)*0.5, xyz, 11, izone3);
	Vector3dToFMatrix((v13+v123)*0.5, xyz, 12, izone3);

	i=13;
	Vector3dToFMatrix(v123_1234, xyz, i++, izone3);
	Vector3dToFMatrix(v23_234, xyz, i++, izone3);
	Vector3dToFMatrix((v3+v34)*0.5, xyz, i++, izone3);
	Vector3dToFMatrix((v13+v134)*0.5, xyz, i++, izone3);	

	Vector3dToFMatrix(v1234_234, xyz, i++, izone3);
	Vector3dToFMatrix(v234_34, xyz, i++, izone3);
	Vector3dToFMatrix((v34+v134)*0.5, xyz, i++, izone3);
	Vector3dToFMatrix((v1234+v134)*0.5, xyz, i++, izone3);

	//=====zone4============================
	Vector3dToFMatrix(v14, xyz, 1, izone4);
	Vector3dToFMatrix(v124, xyz, 2, izone4);
	Vector3dToFMatrix(v1234, xyz, 3, izone4);
	Vector3dToFMatrix(v134, xyz, 4, izone4);

	Vector3dToFMatrix(v4, xyz, 5, izone4);
	Vector3dToFMatrix(v24, xyz, 6, izone4);
	Vector3dToFMatrix(v234, xyz, 7, izone4);
	Vector3dToFMatrix(v34, xyz, 8, izone4);

	Vector3dToFMatrix((v124+v14)*0.5, xyz, 9, izone4);
	Vector3dToFMatrix(v1234_124, xyz, 10, izone4);
	Vector3dToFMatrix((v134+v1234)*0.5, xyz, 11, izone4);
	Vector3dToFMatrix((v134+v14)*0.5, xyz, 12, izone4);

	i=13;
	Vector3dToFMatrix((v14+v4)*0.5, xyz, i++, izone4);
	Vector3dToFMatrix((v24+v124)*0.5, xyz, i++, izone4);
	Vector3dToFMatrix(v1234_234, xyz, i++, izone4);
	Vector3dToFMatrix((v34+v134)*0.5, xyz, i++, izone4);	

	Vector3dToFMatrix((v4+v24)*0.5, xyz, i++, izone4);
	Vector3dToFMatrix(v24_234, xyz, i++, izone4);
	Vector3dToFMatrix(v234_34, xyz, i++, izone4);
	Vector3dToFMatrix((v34+v4)*0.5, xyz, i++, izone4);

    //for vertices 21--26
    i=21;
    //Vector3d vc1=(v14+v124+v1234+v134)*0.25;
	//Vector3dToFMatrix(vc1, xyz, i++, izone4);    //21
    Vector3d vc2=v234*0.5+v4*0.5;
	Vector3dToFMatrix(vc2, xyz, i++, izone4);    //22
/*
    Vector3d vc3=0; //(v14+v124+v24+v4)*0.25;
	Vector3dToFMatrix(vc3, xyz, i++, izone4);    //23
    Vector3d vc4=0; //(v134+v1234+v234+v34)*0.25;
	Vector3dToFMatrix(vc4, xyz, i++, izone4);    //24

    Vector3d vc5=0; //(v14+v134+v34+v4)*0.25;
	Vector3dToFMatrix(vc5, xyz, i++, izone4);    //25
    Vector3d vc6=0; //(v124+v1234+v234+v24)*0.25;
	Vector3dToFMatrix(vc6, xyz, i++, izone4);    //26
*/
}


static inline void 
_fixConeVertices(MV3 & v, const int nx, const int ny, const int nz, const FREAL R, const FREAL H)
{
    Vector3d pt;
    for (int j=2; j<ny; j++){
        for (int i=2; i<nx; i++){
            Vector3d *p = &v(i,j,nz);
            projectPointOnCone(*p, R, H, pt);
            *p = pt;
        }
    }
}


void coneMesher(const int nsplitw, const int nsplith, M1& alpha, const FREAL& r, const FREAL &h, MV3& vertex)
{
    int i;
	M1 xyz0(3); 
	M3 xyz(3,28,4), dirc(3,3,1);
    M2 xyzo(3,6), sca(3,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4), nzsti(4), itype(4);
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=itype(4)=1;

	const int NX=(nsplitw+1)/2+1;
	const int NZ=(nsplith+1)/2+1;
	const int NX2 = NX*2-1; 
	const int NZ2 = NZ*2-1;
	nzsti(1) = 1;
	nzsti(2) = nzsti(1)+NX;
	nzsti(3) = nzsti(2)+NX;
	nzsti(4) = nzsti(3)+NX;
	vertex.reSize(NX*4, NX, NZ); 

	const int izn = 1;
	ixstr(izn)=iystr(izn)=izstr(izn)=1;
	ixend(izn)=NX2;
    iyend(izn)=NX2;
    izend(izn)=NZ2;
    const int lengx=ixend(izn)-ixstr(izn);
    const int lengy=iyend(izn)-iystr(izn);
    const int lengz=izend(izn)-izstr(izn);
    ixend(izn)=ixstr(izn)+lengx/2;
    iyend(izn)=iystr(izn)+lengy/2;
    izend(izn)=izstr(izn)+lengz/2;

	const int izn1 = izn+1;
    ixstr(izn1)=ixstr(izn)+lengx/2;
    ixend(izn1)=ixstr(izn)+lengx;
    iystr(izn1)=iystr(izn);
    iyend(izn1)=iystr(izn)+lengy/2;
    izstr(izn1)=izstr(izn);
    izend(izn1)=izstr(izn)+lengz/2;

	const int izn2 = izn+2;
    ixstr(izn2)=ixstr(izn)+lengx/2;
    ixend(izn2)=ixstr(izn)+lengx;
    iystr(izn2)=iystr(izn)+lengy/2;
    iyend(izn2)=iystr(izn)+lengy;
    izstr(izn2)=izstr(izn);
    izend(izn2)=izstr(izn)+lengz/2;

	const int izn3 = izn+3;
    ixstr(izn3)=ixstr(izn);
    ixend(izn3)=ixstr(izn)+lengx/2;
    iystr(izn3)=iystr(izn);
    iyend(izn3)=iystr(izn)+lengy/2;
    izstr(izn3)=izstr(izn)+lengz/2;
    izend(izn3)=izstr(izn)+lengz;

    //init vertices in the 4 regions
	setControlVertices(alpha(1), alpha(2), r, h, xyz);

	//mesh for each zone
	{
		const int iz=1;
		MV3 mat=vertex(nzsti(iz));
	    mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	} 
	for (i=2; i<=3; i++){ 
		const int iz=i;
		MV3 mat=vertex(nzsti(iz));
		msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}
	{ 
		const int iz=4;
		MV3 mat=vertex(nzsti(iz));
		msh21zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
        const int nx = ixend(iz)-ixstr(iz)+1;
        const int ny = iyend(iz)-iystr(iz)+1;
        const int nz = izend(iz)-izstr(iz)+1;
        _fixConeVertices(mat, nx, ny, nz, r, h);
	}
}


//=========================PRISM=======================================
void prismMesher(M2 & xyzo, const FREAL h, const int nArcDiv, MV3& vertex)
{
//      dimension ixstr(1),ixend(1),iystr(1),iyend(1), izstr(1),izend(1)
//        dimension nzsti(1),nzstj(1),nzstk(1)
//      dimension xyz(3,27,1) dimension x(1),y(1),z(1)
//      dimension itype(1),dirc(3,3,1),xyz0(3,1),sca(3,1)
//      dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
//ixstr,ixend,iystr,iyend,izstr,izend,
//nzsti dirc,xyz0,sca,
	M1 xyz0(3); 
	M3 xyz(3,27,4), dirc(3,3,1);
    M2 xyz1(3,28), xyz2(3,19), sca(3,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4), nzsti(4), itype(4);
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=itype(4)=1;

	ixstr(1)=iystr(1)=izstr(1)=1;
	ixend(1)=iyend(1)=nArcDiv+1;
	izend(1)=1;
	const int izn = 1;
    const int lengx=ixend(izn)-ixstr(izn);
    const int lengy=iyend(izn)-iystr(izn);
    const int lengz=izend(izn)-izstr(izn);
    ixstr(izn)=ixstr(izn);
    ixend(izn)=ixstr(izn)+lengx/2;
    iystr(izn)=iystr(izn);
    iyend(izn)=iystr(izn)+lengy/2;
    izstr(izn)=izstr(izn);
    izend(izn)=izend(izn);
	const int izn1=izn+1;
    ixstr(izn1)=ixstr(izn)+lengx/2;
    ixend(izn1)=ixstr(izn)+lengx;
    iystr(izn1)=iystr(izn);
    iyend(izn1)=iystr(izn)+lengy/2;
    izstr(izn1)=izstr(izn);
    izend(izn1)=izend(izn);
	const int izn2=izn+2;
    ixstr(izn2)=ixstr(izn)+lengx/2;
    ixend(izn2)=ixstr(izn)+lengx;
    iystr(izn2)=iystr(izn)+lengy/2;
    iyend(izn2)=iystr(izn)+lengy;
    izstr(izn2)=izstr(izn);
    izend(izn2)=izend(izn);

	const int nx=ixend(1)-ixstr(1)+1;
	const int nz=izend(1)-izstr(1)+1;
	nzsti(1)=1;
	nzsti(2)=nzsti(1)+nx;
	nzsti(3)=nzsti(2)+nx;
	vertex.reSize(nx*3, nx, nz);

	int k, l;
	for (l=1; l<=6; l++)
		for (k=1; k<=3; k++)
			xyz(k,l,izn)=xyzo(k,l);
	for (k=1; k<=3; k++){
		xyz1(k,1)=(xyzo(k,1)+xyzo(k,2)+xyzo(k,3))/3;
		xyz2(k,1)=(xyzo(k,4)+xyzo(k,5)+xyzo(k,6))/3;
	}
	for (k=1; k<=3; k++){
		xyz1(k,2)=(xyzo(k,1)+xyzo(k,2))/2;
		xyz2(k,2)=(xyzo(k,4)+xyzo(k,5))/2;
	}
	for (k=1; k<=3; k++){
		xyz1(k,3)=0;
		xyz2(k,3)=0;
		for (l=2; l<=3; l++){
			xyz1(k,3)=xyz1(k,3)+xyzo(k,l);
			xyz2(k,3)=xyz2(k,3)+xyzo(k,l+3);
		}
		xyz1(k,3)=xyz1(k,3)/2;
		xyz2(k,3)=xyz2(k,3)/2;
	}

	for (k=1; k<=3; k++){
		xyz1(k,4)=0;
		xyz2(k,4)=0;
		for (l=3; l>=1; l+=-2){
			//do 80 l=3,1,-2
			xyz1(k,4)=xyz1(k,4)+xyzo(k,l);
			xyz2(k,4)=xyz2(k,4)+xyzo(k,l+3);
		}
		xyz1(k,4)=xyz1(k,4)/2;
		xyz2(k,4)=xyz2(k,4)/2;
	}

	for (k=1; k<=3; k++){
		xyz(k,1,izn)=xyzo(k,1);
		xyz(k,2,izn)=xyz1(k,2);
		xyz(k,3,izn)=xyz1(k,1);
		xyz(k,4,izn)=xyz1(k,4);
		xyz(k,5,izn)=xyzo(k,4);
		xyz(k,6,izn)=xyz2(k,2);
		xyz(k,7,izn)=xyz2(k,1);
		xyz(k,8,izn)=xyz2(k,4);
	}

	for (k=1; k<=3; k++){
		xyz(k,1,izn1)=xyz1(k,2);
		xyz(k,2,izn1)=xyzo(k,2);
		xyz(k,3,izn1)=xyz1(k,3);
		xyz(k,4,izn1)=xyz1(k,1);
		xyz(k,5,izn1)=xyz2(k,2);
		xyz(k,6,izn1)=xyzo(k,5);
		xyz(k,7,izn1)=xyz2(k,3);
		xyz(k,8,izn1)=xyz2(k,1);
	}

	for (k=1; k<=3; k++){
		xyz(k,1,izn2)=xyz1(k,1);
		xyz(k,2,izn2)=xyz1(k,3);
		xyz(k,3,izn2)=xyzo(k,3);
		xyz(k,4,izn2)=xyz1(k,4);
		xyz(k,5,izn2)=xyz2(k,1);
		xyz(k,6,izn2)=xyz2(k,3);
		xyz(k,7,izn2)=xyzo(k,6);
		xyz(k,8,izn2)=xyz2(k,4);
	}

	for (int iz=izn; iz<=1; iz++){
		MV3 mat=vertex(nzsti(iz));
        mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}

}


void prismMesher(const Vector3d v[6], const int nArcDiv, MV3& vertex)
{
	M3 xyz(3,27,4);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4), nzsti(4), itype(4);
	itype(1)=itype(2)=itype(3)=itype(4)=1;

    const int ndiv = ((nArcDiv+1)/2)*2;
	ixstr(1)=iystr(1)=izstr(1)=1;
	ixend(1)=iyend(1)=ndiv+1;
	izend(1)=1;
	const int izn = 1;
    const int lengx=ixend(izn)-ixstr(izn);
    const int lengy=iyend(izn)-iystr(izn);
    const int lengz=izend(izn)-izstr(izn);
    ixstr(izn)=ixstr(izn);
    ixend(izn)=ixstr(izn)+lengx/2;
    iystr(izn)=iystr(izn);
    iyend(izn)=iystr(izn)+lengy/2;
    izstr(izn)=izstr(izn);
    izend(izn)=izend(izn);
	const int izn1=izn+1;
    ixstr(izn1)=ixstr(izn)+lengx/2;
    ixend(izn1)=ixstr(izn)+lengx;
    iystr(izn1)=iystr(izn);
    iyend(izn1)=iystr(izn)+lengy/2;
    izstr(izn1)=izstr(izn);
    izend(izn1)=izend(izn);
	const int izn2=izn+2;
    ixstr(izn2)=ixstr(izn)+lengx/2;
    ixend(izn2)=ixstr(izn)+lengx;
    iystr(izn2)=iystr(izn)+lengy/2;
    iyend(izn2)=iystr(izn)+lengy;
    izstr(izn2)=izstr(izn);
    izend(izn2)=izend(izn);

	const int nx=ixend(1)-ixstr(1)+1;
	const int nz=izend(1)-izstr(1)+1;
	nzsti(1)=1;
	nzsti(2)=nzsti(1)+nx;
	nzsti(3)=nzsti(2)+nx;
	vertex.reSize(nx*3, nx, nz);

	//zone 1
	const Vector3d cc = (v[0]+v[1]+v[2]+v[3]+v[4]+v[5])*(1.0/6);
	Vector3dToFMatrix(v[0], xyz, 1, izn);
	Vector3dToFMatrix(v[1], xyz, 2, izn);
	Vector3dToFMatrix(v[2], xyz, 3, izn);
	Vector3dToFMatrix(cc, xyz, 4, izn);
	Vector3dToFMatrix(v[0], xyz, 5, izn);
	Vector3dToFMatrix(v[1], xyz, 6, izn);
	Vector3dToFMatrix(v[2], xyz, 7, izn);
	Vector3dToFMatrix(cc, xyz, 8, izn);
	
	//zone 2
	Vector3dToFMatrix(v[2], xyz, 1, izn1);
	Vector3dToFMatrix(v[3], xyz, 2, izn1);
	Vector3dToFMatrix(v[4], xyz, 3, izn1);
	Vector3dToFMatrix(cc, xyz, 4, izn1);
	Vector3dToFMatrix(v[2], xyz, 5, izn1);
	Vector3dToFMatrix(v[3], xyz, 6, izn1);
	Vector3dToFMatrix(v[4], xyz, 7, izn1);
	Vector3dToFMatrix(cc, xyz, 8, izn1);

	//zone 3
	Vector3dToFMatrix(v[4], xyz, 1, izn2);
	Vector3dToFMatrix(v[5], xyz, 2, izn2);
	Vector3dToFMatrix(v[0], xyz, 3, izn2);
	Vector3dToFMatrix(cc, xyz, 4, izn2);
	Vector3dToFMatrix(v[4], xyz, 5, izn2);
	Vector3dToFMatrix(v[5], xyz, 6, izn2);
	Vector3dToFMatrix(v[0], xyz, 7, izn2);
	Vector3dToFMatrix(cc, xyz, 8, izn2);

	for (int iz=izn; iz<=3; iz++){
		MV3 mat=vertex(nzsti(iz));
        mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}
}


//===============================Directly call msh20zn==========================
void msh20zn(const Vector3d v[20], const int nsx, const int nsy, const int nsz, MV3& vertex)
{
	M1 xyz0(3); 
	M3 xyz(3,20,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4), itype(4);

	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=itype(4)=1;
	const int NX=nsx+1;
	const int NY=nsy+1;
	const int NZ=nsz+1;
	vertex.reSize(NX, NY, NZ); 
	const int izn = 1;
	ixstr(izn)=iystr(izn)=izstr(izn)=1;
	ixend(izn)=NX; iyend(izn)=NY; izend(izn)=NZ;

    for (int i=0; i<20; i++) 
        Vector3dToFMatrix(v[i], xyz, i+1, izn);
	msh20zn(vertex,ixstr,ixend,iystr,iyend,izstr,izend,xyz,izn,itype);
}


