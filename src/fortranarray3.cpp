//fortranarray.cpp

#include "fortranarray.h"
#include "shapefunc.h"


#define call 
const FREAL FQUARTER=0.25;
const FREAL FHALF=0.5;


//=========================2D triangle=======================================

void triangleMesher(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, const int nArcDiv, MV3& vertex)
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

	const Vector3d v123 = (v1+v2+v3)*(1.0/3);
	const Vector3d v12 = (v1+v2)*0.5;
	const Vector3d v23 = (v2+v3)*0.5;
	const Vector3d v13 = (v1+v3)*0.5;

	//zone 1
	Vector3dToFMatrix(v1, xyz, 1, izn);
	Vector3dToFMatrix(v12, xyz, 2, izn);
	Vector3dToFMatrix(v123, xyz, 3, izn);
	Vector3dToFMatrix(v13, xyz, 4, izn);
	Vector3dToFMatrix(v1, xyz, 5, izn);
	Vector3dToFMatrix(v12, xyz, 6, izn);
	Vector3dToFMatrix(v123, xyz, 7, izn);
	Vector3dToFMatrix(v13, xyz, 8, izn);
	
	//zone 2
	Vector3dToFMatrix(v2, xyz, 1, izn1);
	Vector3dToFMatrix(v23, xyz, 2, izn1);
	Vector3dToFMatrix(v123, xyz, 3, izn1);
	Vector3dToFMatrix(v12, xyz, 4, izn1);
	Vector3dToFMatrix(v2, xyz, 5, izn1);
	Vector3dToFMatrix(v23, xyz, 6, izn1);
	Vector3dToFMatrix(v123, xyz, 7, izn1);
	Vector3dToFMatrix(v12, xyz, 8, izn1);

	//zone 3
	Vector3dToFMatrix(v3, xyz, 1, izn2);
	Vector3dToFMatrix(v13, xyz, 2, izn2);
	Vector3dToFMatrix(v123, xyz, 3, izn2);
	Vector3dToFMatrix(v23, xyz, 4, izn2);
	Vector3dToFMatrix(v3, xyz, 5, izn2);
	Vector3dToFMatrix(v13, xyz, 6, izn2);
	Vector3dToFMatrix(v123, xyz, 7, izn2);
	Vector3dToFMatrix(v23, xyz, 8, izn2);

	for (int iz=izn; iz<=3; iz++){
		MV3 mat=vertex(nzsti(iz));
        mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}
}

