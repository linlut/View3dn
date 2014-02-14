//fortranarray.cpp

#include "fortranarray.h"
#include "shapefunc.h"


#define call 
const FREAL FQUARTER=0.25;
const FREAL FHALF=0.5;

//===========================BRICK===========================================

static void 
brick(Vector3d *vertex, const int nx, const int ny, const int nz, M2& xyz, const int izn=1)
{
    FREAL shape[9];      
	const int xleng=nx-1;
    const int yleng=ny-1;
    const int zleng=nz-1;
	const int izstr = 1;
	const int iystr = 1;
	const int ixstr = 1;
    Vector3d vv[9];

    int l, c=0;
    for (l=1; l<=8; l++)
        vv[l].x = xyz(1,l), vv[l].y = xyz(2,l), vv[l].z = xyz(3,l);

	for (int k=0; k<=zleng; k++){
		const int kk=k+izstr;
		const FREAL ztasp=-1+k*2.0/zleng;
		for (int j=0; j<=yleng; j++){
			const int jj=j+iystr;
			const FREAL etasp=-1+j*2.0/yleng;
			for (int i=0; i<=xleng; i++, c++){
				const int ii=i+ixstr;
				const FREAL exisp=-1+i*2.0/xleng;
				SHAPEGRD8_NODEDRI(shape, exisp, etasp, ztasp); 
				FREAL x0=0, y0=0, z0=0;
				for (l=1; l<=8; l++){
					const FREAL shapel = shape[l];
                    if (shapel!=0){
                        const Vector3d *p = &vv[l];
					    x0 += shapel * p->x;
					    y0 += shapel * p->y;
					    z0 += shapel * p->z;
                    }
				}
				vertex[c]=Vector3d(x0,y0,z0);
			}
		}
	}
}


void meshSlantBrickVertices(const Vector3d cubevert[8], const int nx, const int ny, const int nz, Vector3d * &meshvert, int &nv)
{
	if (nx<1 || ny<1 || nz<1) assert(0);
	M2 xyz(3,8);
	int i, k;

	for (i=0; i<8; i++){
	    k=i+1;
		const Vector3d *v = &cubevert[i];
		xyz(1,k) = v->x,  xyz(2,k) = v->y, xyz(3,k) = v->z;
	}

	//create the brick elements
	nv = nx*ny*nz;
	meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);
	brick(meshvert, nx, ny, nz, xyz);
}


//===========================SPHERE===========================================
static void 
sphere(M3 &xyz, const int izn, M1& xyz0, M1& alpha, M1& beta, const FREAL& r, M2& xyzo, M2& xyz1, M2& xyz2)
{
//dimension xyz(3,27,1),xyz0(3),beta(2),alpha(2)
//dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
//c==	generate node coordinates
//c	node 1
	xyzo(1,1)=xyz0(1), 
	xyzo(2,1)=xyz0(2), 
	xyzo(3,1)=xyz0(3);
//c	node 2
    FREAL u=alpha(1);
    FREAL v=beta(2);
    xyzcart(u,v,r,xyzo,2,3,xyz0);
//c	node 3
    u=alpha(2);
    v=beta(2);
    xyzcart(u,v,r,xyzo,3,3,xyz0);
//c	node 4
    u=alpha(2);
    v=beta(1);
    xyzcart(u,v,r,xyzo,4,3,xyz0);
	int l, k;
    for (k=1; k<=3; k++)
		for (l=1; l<=4; l++)
			xyz(k,l,izn)=xyzo(k,l);
//c==   generate midpoint coordinates which are nodal coordinates for 
//c==   each sub-zone
//c     midpoint of surface 1-2-3 (c1)
	const FREAL alpha12 = (alpha(2)+alpha(1))*FHALF;
	const FREAL beta12 = (beta(2)+beta(1))*FHALF;
    u=alpha12;
    v=beta(2);
    FREAL rh=r*0.62;
    xyzcart(u,v,rh,xyz1,1,3,xyz0);
//c     midpoint of side 1-2 (c2)
    u=alpha(1);
    v=beta(2);
    rh=r/2;
    xyzcart(u,v,rh,xyz1,2,3,xyz0);
//c     midpoint of side 2-3 (c3)
    u=alpha12;
    v=beta(2);
    rh=r;
    xyzcart(u,v,rh,xyz1,3,3,xyz0);
//c     midpoint of side 3-1 (c4)
    u=alpha(2);
    v=beta(2);
    rh=r/2;
    xyzcart(u,v,rh,xyz1,4,3,xyz0);
//c     midpoint of body 1-2-3-4 (c1')
    u=alpha12;
    v=beta12;
    rh=r*0.62;
    xyzcart(u,v,rh,xyz2,1,3,xyz0);
//c     midpoint of surface 1-2-4 (c2')
    u=alpha(1);
    v=beta12;
    rh=r*0.62;
    xyzcart(u,v,rh,xyz2,2,3,xyz0);
//c     midpoint of surface 2-3-4 (c3')
    u=alpha12;
    v=0.62*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz2,3,3,xyz0);
//c     midpoint of surface 1-3-4 (c4')
    u=alpha(2);
    v=beta12;
    rh=r*0.62;
    xyzcart(u,v,rh,xyz2,4,3,xyz0);
//c     midpoint of side 2-4 (c5')
    u=alpha(1);
    v=beta12;
    rh=r;
    xyzcart(u,v,rh,xyz2,5,3,xyz0);
//c     midpoint of side 3-4 (c6')
    u=alpha(2);
    v=beta12;
    rh=r;
    xyzcart(u,v,rh,xyz2,6,3,xyz0);
//c     midpoint of side 1-4 (c7')
    u=alpha(1);
    v=beta(1);
    rh=r/2;
    xyzcart(u,v,rh,xyz2,7,3,xyz0);
//c==   generate midpoint coordinates for sub-zones
//c*    sub-zone 2
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
    xyzcart(u,v,rh,xyz1,6,3,xyz0);
//c     midpoint of side 2-c5'
    u=alpha(1);
    v=0.75*(beta(2)-beta(1))+beta(1);
    rh=r;
    xyzcart(u,v,rh,xyz1,10,3,xyz0);
//c     midpoint of side c3-c3'
    u=alpha12;
    v=0.81*(beta(2)-beta(1))+beta(1);
    rh=r;
    xyzcart(u,v,rh,xyz1,11,3,xyz0);
//c     midpoint of side c5'-c3'
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    rh=r;
    xyzcart(u,v,rh,xyz1,14,3,xyz0);
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
    xyzcart(u,v,rh,xyz1,18,3,xyz0);
//c     midpoint of side c3-c3'
	u=alpha12;
    v=0.81*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz1,22,3,xyz0);
//c     midpoint of side 3-c6'
    u=alpha(2);
    v=0.75*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz1,23,3,xyz0);
//c     midpoint of side c3'-c6'
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz1,26,3,xyz0);
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
    xyzcart(u,v,r,xyz2,17,3,xyz0);
//c     midpoint of side c5'-4
    u=alpha(1);
    v=0.25*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz2,16,3,xyz0);
//c     midpoint of side c3'-c6'
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0.551*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz2,18,3,xyz0);
//c     midpoint of side 4-c6'
    u=alpha(2);
    v=0.25*(beta(2)-beta(1))+beta(1);
    xyzcart(u,v,r,xyz2,19,3,xyz0);
}


static inline void 
_decideSplitNumberXYZ(const FREAL& a0, const FREAL& a1, const FREAL& b0, const FREAL& b1, const int nsplit, int &nx, int &ny, int& nz)
{
	const int divz = (nsplit+2)/2;
	const int divx = divz; //divz*fabs((a1-a0)/(b1-b0))+0.5; 
	const int divy = divz;
	nx = divx;
	ny = divy;
	nz = divz;
}


void _setSphControlVertices(M3& xyz, M2& xyzo, M2& xyz1, M2& xyz2)
{
	int k;
	const int izn=1, izn1=2, izn2=3, izn3=4;
	for (k=1; k<=3; k++){		//c==	zone 1
		xyz(k,1,izn)=xyzo(k,1);
		xyz(k,2,izn)=xyz1(k,2);
		xyz(k,3,izn)=xyz1(k,1);
		xyz(k,4,izn)=xyz1(k,4);
		xyz(k,5,izn)=xyz2(k,7);
		xyz(k,6,izn)=xyz2(k,2);
		xyz(k,7,izn)=xyz2(k,1);
		xyz(k,8,izn)=xyz2(k,4);
	}
	for (k=1; k<=3; k++){		//c==	zone 2
      xyz(k,1,izn1)=xyz1(k,2);
      xyz(k,2,izn1)=xyzo(k,2);
      xyz(k,3,izn1)=xyz1(k,3);
      xyz(k,4,izn1)=xyz1(k,1);
      xyz(k,5,izn1)=xyz2(k,2);
      xyz(k,6,izn1)=xyz2(k,5);
      xyz(k,7,izn1)=xyz2(k,3);
      xyz(k,8,izn1)=xyz2(k,1);
      xyz(k,9,izn1)=xyz1(k,5);
      xyz(k,10,izn1)=xyz1(k,6);
      xyz(k,11,izn1)=xyz1(k,7);
      xyz(k,12,izn1)=xyz1(k,8);
      xyz(k,13,izn1)=xyz1(k,9);
      xyz(k,14,izn1)=xyz1(k,10);
      xyz(k,15,izn1)=xyz1(k,11);
      xyz(k,16,izn1)=xyz1(k,12);
      xyz(k,17,izn1)=xyz1(k,13);
      xyz(k,18,izn1)=xyz1(k,14);
      xyz(k,19,izn1)=xyz1(k,15);
      xyz(k,20,izn1)=xyz1(k,16);
	}
    for (k=1; k<=3; k++){		//c==	zone 3
      xyz(k,1,izn2)=xyz1(k,1);
      xyz(k,2,izn2)=xyz1(k,3);
      xyz(k,3,izn2)=xyzo(k,3);
      xyz(k,4,izn2)=xyz1(k,4);
      xyz(k,5,izn2)=xyz2(k,1);
      xyz(k,6,izn2)=xyz2(k,3);
      xyz(k,7,izn2)=xyz2(k,6);
      xyz(k,8,izn2)=xyz2(k,4);
      xyz(k,9,izn2)=xyz1(k,17);
      xyz(k,10,izn2)=xyz1(k,18);
      xyz(k,11,izn2)=xyz1(k,19);
      xyz(k,12,izn2)=xyz1(k,20);
      xyz(k,13,izn2)=xyz1(k,21);
      xyz(k,14,izn2)=xyz1(k,22);
      xyz(k,15,izn2)=xyz1(k,23);
      xyz(k,16,izn2)=xyz1(k,24);
      xyz(k,17,izn2)=xyz1(k,25);
      xyz(k,18,izn2)=xyz1(k,26);
      xyz(k,19,izn2)=xyz1(k,27);
      xyz(k,20,izn2)=xyz1(k,28);
	}
	for (k=1; k<=3; k++){		//c==	zone 4
      xyz(k,1,izn3)=xyz2(k,7);
      xyz(k,2,izn3)=xyz2(k,2);
      xyz(k,3,izn3)=xyz2(k,1);
      xyz(k,4,izn3)=xyz2(k,4);
      xyz(k,5,izn3)=xyzo(k,4);
      xyz(k,6,izn3)=xyz2(k,5);
      xyz(k,7,izn3)=xyz2(k,3);
      xyz(k,8,izn3)=xyz2(k,6);
      xyz(k,9,izn3)=xyz2(k,8);
      xyz(k,10,izn3)=xyz2(k,9);
      xyz(k,11,izn3)=xyz2(k,10);
      xyz(k,12,izn3)=xyz2(k,11);
      xyz(k,13,izn3)=xyz2(k,12);
      xyz(k,14,izn3)=xyz2(k,13);
      xyz(k,15,izn3)=xyz2(k,14);
      xyz(k,16,izn3)=xyz2(k,15);
      xyz(k,17,izn3)=xyz2(k,16);
      xyz(k,18,izn3)=xyz2(k,17);
      xyz(k,19,izn3)=xyz2(k,18);
      xyz(k,20,izn3)=xyz2(k,19);
	}
}

static void 
validBoundaryVertices(const double r, MV3 & vert, const int x1, const int x2, 
    const int y1, const int y2, const int z1, const int z2)
{
    for (int k=z1; k<=z2; k++){
        for (int j=y1; j<=y2; j++){
            for (int i=x1; i<=x2; i++){
                Vector3d& v=vert(i, j, k);
                v.normalize();
                v*= r;
            }
        }
    }
}

void sprmshzn(const int nsplit, M1& alpha, M1& beta, const FREAL& r, MV3& vertex)
{
	M1 xyz0(3); 
	M3 xyz(3,27,4), dirc(3,3,1);
    M2 xyzo(3,6), xyz1(3,28), xyz2(3,19), sca(3,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4), nzsti(4), itype(4);
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=itype(4)=1;

	int NX, NY, NZ, NX4, NX2, NY2, NZ2;
	_decideSplitNumberXYZ(alpha(1), alpha(2), beta(1), beta(2), nsplit, NX, NY, NZ);
	NX4 = NX*4;
	NX2 = NX*2-1; NY2 = NY*2-1; NZ2 = NZ*2-1;
	vertex.reSize(NX4, NY, NZ), 
	nzsti(1) = 1;
	nzsti(2) = nzsti(1)+NX;
	nzsti(3) = nzsti(2)+NX;
	nzsti(4) = nzsti(3)+NX;

	const int izn = 1;
	ixstr(izn)=iystr(izn)=izstr(izn)=1;
	ixend(izn)=NX2;
    iyend(izn)=NY2;
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

	//generate control vertices
    sphere(xyz,izn,xyz0,alpha,beta,r,xyzo,xyz1,xyz2);
	//assign vertices to zones
	_setSphControlVertices(xyz, xyzo, xyz1, xyz2);
	//mesh for each zone
	{
		const int iz=1;
		MV3 mat=vertex(nzsti(iz));
		mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}

    int iz=2, nx, ny, nz;
	MV3 mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, nx, nx, 1, ny, 1, nz); 

    iz=3;
	mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, nx, nx, 1, ny, 1, nz); 

    iz=4;
	mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, 1, nx, 1, ny, nz, nz); 
}



void meshSphereVertices(const Vector3d& ct, const FREAL r, 
						const FREAL& a0, const FREAL& a1, 
						const FREAL& b0, const FREAL& b1, 
						const int nx1, const int nx2, 
						Vector3d * &meshvert, int &nv)
{
	assert(nx1==1);
	MV3 vertex;
	M1 alpha(2), beta(2);

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	beta(1)=b0, beta(2)=b1;
	sprmshzn(nx2, alpha, beta, r, vertex);

	int nx, ny, nz, c = 0;
	vertex.getMatrixDimension(nx, ny, nz);
	nv = vertex.getMatrixSize();	
	meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);
	for (int k=1; k<=nz; k++){
		for (int j=1; j<=ny; j++){
			for (int i=1; i<=nx; i++, c++){
				//copy to row major C-style array, no matter the matrix is 
				//row major or column major
				meshvert[c] = vertex(i, j, k)+ct;
			}
		}
	}
	
}


//===================CYLINDER================================

static void 
colum(M3& xyz, const int izn, M1& xyz0, const FREAL& h, M1& alpha, const FREAL &r, M2& xyzo, M2& xyz1, M2& xyz2)
{
	//dimension xyz(3,27,1),xyz0(3),h(2),alpha(2)
    //dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
	FREAL u, v, rh;
	int k, l;
//c	node 1
    for (k=1; k<=3; k++)
		xyzo(k,1)=xyz0(k);
//c	node 2
    u=alpha(1);
    v=0;
    xyzcart(u,v,r,xyzo,2,2,xyz0);
//c	node 3
	u=alpha(2);
    v=0;
    xyzcart(u,v,r,xyzo,3,2,xyz0);
//c	node 4
    v=h;
    xyzcart(u,v,0,xyzo,4,2,xyz0);
//c	node 5
    u=alpha(1);
    v=h;
    xyzcart(u,v,r,xyzo,5,2,xyz0);
//c	node 6
    u=alpha(2);
    v=h;
    xyzcart(u,v,r,xyzo,6,2,xyz0);

    for (k=1; k<=3; k++)
		for (l=1; l<=6; l++)
			xyz(k,l,izn)=xyzo(k,l);

//c     midpoint
//c     midpoint on b. surface
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0;
    rh=r*0.62;
    xyzcart(u,v,rh,xyz1,1,2,xyz0);
//c     midpoint on t. surface
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=h;
    rh=r*0.62;
    xyzcart(u,v,rh,xyz2,1,2,xyz0);
//c     midpoint of side 1-2
    u=alpha(1);
    v=0;
    rh=r/2;
    xyzcart(u,v,rh,xyz1,2,2,xyz0);
//c     midpoint of side 2-3
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0;
    xyzcart(u,v,r,xyz1,3,2,xyz0);
//c     midpoint of side 3-1
    u=alpha(2);
    v=0;
    rh=r/2;
    xyzcart(u,v,rh,xyz1,4,2,xyz0);
//c     midpoint of side 4-5
    u=alpha(1);
    v=h;
    rh=r/2;
    xyzcart(u,v,rh,xyz2,2,2,xyz0);
//c     midpoint of side 5-6
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=h;
    rh=r;
    xyzcart(u,v,rh,xyz2,3,2,xyz0);
//c     midpoint of side 6-4
    u=alpha(2);
    v=h;
    rh=r/2;
    xyzcart(u,v,rh,xyz2,4,2,xyz0);
}


static void
midside(M3 &xyz, const int izn, M1& xyz0, const FREAL& h, M1& alpha, const FREAL& r, M2 &xyzo, M2& xyz1, M2& xyz2)
{
//     dimension xyz(3,27,1),xyz0(3),h(2),alpha(2)
//      dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
//c==	sub-zone 2
//c     midpoint
//c     midpoint of side c2-2
	FREAL rh, u, v;
	u=alpha(1);
    v=0;
    rh=3.0*r/4.0;
    xyzcart(u,v,rh,xyz1,5,2,xyz0);
//c     midpoint of side 2-c3
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0;
    xyzcart(u,v,r,xyz1,6,2,xyz0);
//c     midpoint of side c3-c1
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0;
    rh=0.81*r;
    xyzcart(u,v,rh,xyz1,7,2,xyz0);
//c     midpoint of side c1-c2
    xyz1(1,8)=(xyz1(1,1)+xyz1(1,2))*FHALF;
    xyz1(2,8)=(xyz1(2,1)+xyz1(2,2))*FHALF;
    xyz1(3,8)=(xyz1(3,1)+xyz1(3,2))*FHALF;
//c     midpoint of side c2'-5
    u=alpha(1);
    v=h;
    rh=3*r/4;
    call xyzcart(u,v,rh,xyz2,5,2,xyz0);
//c     midpoint of side 5-c3'
    u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=h;
    call xyzcart(u,v,r,xyz2,6,2,xyz0);
//c     midpoint of side c3'-c1'
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=h;
    rh=0.81*r;
    call xyzcart(u,v,rh,xyz2,7,2,xyz0);
//c     midpoint of side c1'-c2'
    xyz2(1,8)=(xyz2(1,1)+xyz2(1,2))*FHALF;
    xyz2(2,8)=(xyz2(2,1)+xyz2(2,2))*FHALF;
    xyz2(3,8)=(xyz2(3,1)+xyz2(3,2))*FHALF;

//c     midpoint of side c2-c2'
    xyz1(1,13)=(xyz1(1,2)+xyz2(1,2))*FHALF;
    xyz1(2,13)=(xyz1(2,2)+xyz2(2,2))*FHALF;
    xyz1(3,13)=(xyz1(3,2)+xyz2(3,2))*FHALF;
//c     midpoint of side 2-5
    xyz1(1,14)=(xyzo(1,2)+xyzo(1,5))*FHALF;
    xyz1(2,14)=(xyzo(2,2)+xyzo(2,5))*FHALF;
    xyz1(3,14)=(xyzo(3,2)+xyzo(3,5))*FHALF;
//c     midpoint of side c3-c3'
    xyz1(1,15)=(xyz1(1,3)+xyz2(1,3))*FHALF;
    xyz1(2,15)=(xyz1(2,3)+xyz2(2,3))*FHALF;
    xyz1(3,15)=(xyz1(3,3)+xyz2(3,3))*FHALF;
//c     midpoint of side c1-c1'
    xyz1(1,16)=(xyz1(1,1)+xyz2(1,1))*FHALF;
    xyz1(2,16)=(xyz1(2,1)+xyz2(2,1))*FHALF;
    xyz1(3,16)=(xyz1(3,1)+xyz2(3,1))*FHALF;
//c==	sub-zone 3
//c     midpoint of side c1-c3
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=0;
    rh=0.81*r;
    xyzcart(u,v,rh,xyz1,9,2,xyz0);
//c     midpoint of side c3-3
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=0;
    rh=r;
    xyzcart(u,v,rh,xyz1,10,2,xyz0);
//c     midpoint of side 3-c4
    u=alpha(2);
    v=0;
    rh=3*r/4;
    xyzcart(u,v,rh,xyz1,11,2,xyz0);
//c     midpoint of side c4-c1
    xyz1(1,12)=(xyz1(1,1)+xyz1(1,4))*FHALF;
    xyz1(2,12)=(xyz1(2,1)+xyz1(2,4))*FHALF;
    xyz1(3,12)=(xyz1(3,1)+xyz1(3,4))*FHALF;
//c     midpoint of side c1'-c3'
    u=(alpha(2)-alpha(1))*FHALF+alpha(1);
    v=h;
    rh=0.81*r;
    call xyzcart(u,v,rh,xyz2,9,2,xyz0);
//c     midpoint of side c3'-6
    u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
    v=h;
    rh=r;
    call xyzcart(u,v,rh,xyz2,10,2,xyz0);
//c     midpoint of side 6-c4'
    u=alpha(2);
    v=h;
    rh=3*r/4;
    call xyzcart(u,v,rh,xyz2,11,2,xyz0);
//c     midpoint of side c4'-c1'
    xyz2(1,12)=(xyz2(1,1)+xyz2(1,4))*FHALF;
    xyz2(2,12)=(xyz2(2,1)+xyz2(2,4))*FHALF;
    xyz2(3,12)=(xyz2(3,1)+xyz2(3,4))*FHALF;
//c     midpoint of side c1-c1'
    xyz2(1,13)=(xyz1(1,1)+xyz2(1,1))*FHALF;
    xyz2(2,13)=(xyz1(2,1)+xyz2(2,1))*FHALF;
    xyz2(3,13)=(xyz1(3,1)+xyz2(3,1))*FHALF;
//c     midpoint of side c3-c3'
    xyz2(1,14)=(xyz1(1,3)+xyz2(1,3))*FHALF;
    xyz2(2,14)=(xyz1(2,3)+xyz2(2,3))*FHALF;
    xyz2(3,14)=(xyz1(3,3)+xyz2(3,3))*FHALF;
//c     midpoint of side 3-6
    xyz2(1,15)=(xyzo(1,3)+xyzo(1,6))*FHALF;
    xyz2(2,15)=(xyzo(2,3)+xyzo(2,6))*FHALF;
    xyz2(3,15)=(xyzo(3,3)+xyzo(3,6))*FHALF;
//c     midpoint of side c4-c4'
    xyz2(1,16)=(xyz1(1,4)+xyz2(1,4))*FHALF;
    xyz2(2,16)=(xyz1(2,4)+xyz2(2,4))*FHALF;
    xyz2(3,16)=(xyz1(3,4)+xyz2(3,4))*FHALF;
}

static void _decideSplitNumberCyl(const FREAL& r, const FREAL& h, const int nsplit, int &nx, int &ny)
{
	const int divx = (nsplit+2)/2;
	const int divy = divx;  
	nx = divx;
	ny = divy;
}

void 
colmshzn(const int nsplit, const FREAL &r, M1& alpha, const FREAL &h, MV3 &vertex)
{
//      dimension h(2),alpha(2)
//      dimension ixstr(1),ixend(1),iystr(1),iyend(1), izstr(1),izend(1)
//      dimension nzsti(1),nzstj(1),nzstk(1)
//      dimension xyz(3,27,1)
//      dimension x(1),y(1),z(1)
//      dimension itype(1),dirc(3,3,1),xyz0(3,1),sca(3,1)
//      dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
	const int NZONE=3;
	M1 xyz0(3); 
	M3 xyz(3,27,4), dirc(3,3,1);
    M2 xyzo(3,6), xyz1(3,28), xyz2(3,19), sca(3,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4);
	IA1 nzsti(NZONE), itype(NZONE);
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=1;

	int NX, NY;
	_decideSplitNumberCyl(r, h, nsplit, NX, NY);
	const int NX3 = 3*NX;
	const int NX2 = NX*2-1;
	const int NY2 = NY*2-1;
	vertex.reSize(NX3, NY, 1), 
	nzsti(1) = 1;
	nzsti(2) = nzsti(1)+NX;
	nzsti(3) = nzsti(2)+NX;

    const int izn=1;
	  ixstr(izn)=1;
	  ixend(izn)=NX2;
      iystr(izn)=1;
      iyend(izn)=NY2;
      izstr(izn)=1;
      izend(izn)=1;
      const int lengx=ixend(izn)-ixstr(izn);
      const int lengy=iyend(izn)-iystr(izn);
      ixstr(izn)=ixstr(izn);
      ixend(izn)=ixstr(izn)+lengx/2;
      iystr(izn)=iystr(izn);
      iyend(izn)=iystr(izn)+lengy/2;

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

	//kall the discretize func.
	call colum(xyz,izn,xyz0,h,alpha,r,xyzo,xyz1,xyz2);

//c	sub-zone 1
	int k;
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
    call midside(xyz,izn,xyz0,h,alpha,r,xyzo,xyz1,xyz2);

//c	sub-zone 2
	for (k=1; k<=3; k++){
		xyz(k,1,izn1)=xyz1(k,2);
		xyz(k,2,izn1)=xyzo(k,2);
		xyz(k,3,izn1)=xyz1(k,3);
		xyz(k,4,izn1)=xyz1(k,1);
		xyz(k,5,izn1)=xyz2(k,2);
		xyz(k,6,izn1)=xyzo(k,5);
		xyz(k,7,izn1)=xyz2(k,3);
		xyz(k,8,izn1)=xyz2(k,1);
		xyz(k,9,izn1)=xyz1(k,5);
		xyz(k,10,izn1)=xyz1(k,6);
		xyz(k,11,izn1)=xyz1(k,7);
		xyz(k,12,izn1)=xyz1(k,8);
		xyz(k,13,izn1)=xyz1(k,13);
		xyz(k,14,izn1)=xyz1(k,14);
		xyz(k,15,izn1)=xyz1(k,15);
		xyz(k,16,izn1)=xyz1(k,16);
		xyz(k,17,izn1)=xyz2(k,5);
		xyz(k,18,izn1)=xyz2(k,6);
		xyz(k,19,izn1)=xyz2(k,7);
		xyz(k,20,izn1)=xyz2(k,8);
	 }

//c	sub-zone 3
	for (k=1; k<=3; k++){
		xyz(k,1,izn2)=xyz1(k,1);
		xyz(k,2,izn2)=xyz1(k,3);
		xyz(k,3,izn2)=xyzo(k,3);
		xyz(k,4,izn2)=xyz1(k,4);
		xyz(k,5,izn2)=xyz2(k,1);
		xyz(k,6,izn2)=xyz2(k,3);
		xyz(k,7,izn2)=xyzo(k,6);
		xyz(k,8,izn2)=xyz2(k,4);
		xyz(k,9,izn2)=xyz1(k,9);
		xyz(k,10,izn2)=xyz1(k,10);
		xyz(k,11,izn2)=xyz1(k,11);
		xyz(k,12,izn2)=xyz1(k,12);
		xyz(k,13,izn2)=xyz2(k,13);
		xyz(k,14,izn2)=xyz2(k,14);
		xyz(k,15,izn2)=xyz2(k,15);
		xyz(k,16,izn2)=xyz2(k,16);
		xyz(k,17,izn2)=xyz2(k,9);
		xyz(k,18,izn2)=xyz2(k,10);
		xyz(k,19,izn2)=xyz2(k,11);
		xyz(k,20,izn2)=xyz2(k,12);
	}
	//zone 1
    const int iz=1;
	MV3 mat=vertex(nzsti(iz));
    mesh8zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	//zone 2, 3
	for (int i=2; i<=3; i++){
		const int iz=i;
		MV3 mat=vertex(nzsti(iz));
		msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
	}
}


void meshCylinderVertices(
		const Vector3d& ct, const FREAL r, const FREAL& a0, const FREAL& a1, const FREAL& h, 
		const int nsplitw, const int nsplith,
		Vector3d * &meshvert, int &nv)
{
	MV3 vertex;
	M1 alpha(2);
	int i, j;

	//create the sphere elements
	alpha(1)=a0, alpha(2)=a1;
	colmshzn(nsplitw, r, alpha, h, vertex);

	int nx, ny, nz, c;
	vertex.getMatrixDimension(nx, ny, nz);
	assert(nz==1);
	const int nsize = nx*ny*nz;
	nv = nsize * (nsplith+1);
	meshvert = new Vector3d[nv];
	assert(meshvert!=NULL);

	//copy the first layer vertices
	c = nsplith*nsize;
	for (int k=1; k<=nz; k++){
		for (j=1; j<=ny; j++){
			for (i=1; i<=nx; i++, c++){
				//copy to row major C-style array, no matter the matrix is 
				//row major or column major
				meshvert[c] = vertex(i, j, k)+ct;
				//printf("Array %d[%d %d %d]=%g, %g %g\n", c, i, j, k, meshvert[c].x, meshvert[c].y, meshvert[c].z);
			}
		}
	}

	//duplicate the rest layers from the first layer
	for (i=0; i<nsplith; i++){
		Vector3d *psrc = &meshvert[nsplith*nsize];
		Vector3d *pdst = &meshvert[i*nsize];
		memcpy(pdst, psrc, nsize*sizeof(Vector3d));
	}
	for (c=j=0; j<nsplith; j++){
		double dz = (h*(nsplith-j))/(double)nsplith;
		float zval = meshvert[c].z-dz;
		for (i=0; i<nsize; i++, c++)
			meshvert[c].z = zval;
	}
}


//==============================================================================
static void 
gshell(M3& xyz, const int izn, M1& xyz0, M1& alpha, M1& beta, 
	   const FREAL& r, const FREAL& h, M2& xyzo, M2& xyz1, M2& xyz2)
{      
	//dimension xyz(3,27,1),xyz0(3),beta(2),alpha(2)
    //dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
//c==	generate node coordinates
//c	node 4
    FREAL u=alpha(1);
    FREAL v=beta(1);
    call xyzcart(u,v,r,xyzo,4,3,xyz0);
//c	node 5
    u=alpha(1);
    v=beta(2);
    call xyzcart(u,v,r,xyzo,5,3,xyz0);
//c	node 6
    u=alpha(2);
    v=beta(2);
    call xyzcart(u,v,r,xyzo,6,3,xyz0);
//c	node 1,2 3
	int k, l;
	for (k=1; k<=3; k++){
		xyzo(k,1)=xyzo(k,4);
		xyzo(k,2)=xyzo(k,5);
		xyzo(k,3)=xyzo(k,6);
	}
    xyzo(3,1)=xyzo(3,4)-h;
    xyzo(3,2)=xyzo(3,5)-h;
    xyzo(3,3)=xyzo(3,6)-h;
    for (k=1; k<=3; k++)
		for (l=1; l<=6; l++)
			xyz(k,l,izn)=xyzo(k,l);
//c==   generate midpoint coordinates which are nodal coordinates for 
//c==   each sub-zone
//c     midpoint on t. surface (c1')
	FREAL rh;
      u=(alpha(2)+alpha(1))*FHALF;
      v=0.62*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,1,3,xyz0);
//c     midpoint of side 4-5 (c2')
      u=alpha(1);
      v=(beta(1)+beta(2))*FHALF;
      rh=r;
      call xyzcart(u,v,rh,xyz2,2,3,xyz0);
//c     midpoint of side 5-6 (c3')
      u=(alpha(2)+alpha(1))*FHALF;
      v=beta(2);
      call xyzcart(u,v,r,xyz2,3,3,xyz0);
//c     midpoint of side 6-4 (c4')
      u=alpha(2);
      v=(beta(1)+beta(2))*FHALF;
      rh=r;
      call xyzcart(u,v,rh,xyz2,4,3,xyz0);
//c     midpoint of side 1-2 (c2), 2-3 (c3), 3-1 (c4)
	for (k=1; k<=3; k++){
		xyz1(k,1)=xyz2(k,1);
		xyz1(k,2)=xyz2(k,2);
		xyz1(k,3)=xyz2(k,3);
		xyz1(k,4)=xyz2(k,4);
	}
      xyz1(3,1)=xyz2(3,1)-h;
      xyz1(3,2)=xyz2(3,2)-h;
      xyz1(3,3)=xyz2(3,3)-h;
      xyz1(3,4)=xyz2(3,4)-h;
//c==   generate midpoint coordinates for sub-zones
//c*    sub-zone 1
//c     midpoint of side 4-c2'
      u=alpha(1);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,5,3,xyz0);
//c     midpoint of side c2'-c1'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      call xyzcart(u,v,r,xyz2,6,3,xyz0);
//c     midpoint of side c1'-c4'
      u=alpha(2)-(alpha(2)-alpha(1))*FQUARTER;
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,7,3,xyz0);
//c     midpoint of side c4'-4
      u=alpha(2);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,8,3,xyz0);
//c     midpoint of side 1-c2,c2-c1,c1-c4,c4-1
	for (k=1; k<=3; k++){
		xyz1(k,5)=xyz2(k,5);
		xyz1(k,6)=xyz2(k,6);
		xyz1(k,7)=xyz2(k,7);
		xyz1(k,8)=xyz2(k,8);
	}
      xyz1(3,5)=xyz2(3,5)-h;
      xyz1(3,6)=xyz2(3,6)-h;
      xyz1(3,7)=xyz2(3,7)-h;
      xyz1(3,8)=xyz2(3,8)-h;
//c     midpoint of side 1-4,c2-c2',c1-c1',c4-c4'
	for (k=1; k<=3; k++){
		xyz1(k,17)=(xyzo(k,1)+xyzo(k,4))*FHALF;
		xyz1(k,18)=(xyz1(k,2)+xyz2(k,2))*FHALF;
		xyz1(k,19)=(xyz1(k,1)+xyz2(k,1))*FHALF;
		xyz1(k,20)=(xyz1(k,4)+xyz2(k,4))*FHALF;
	}
//c*    sub-zone 2
//c     midpoint of side c2'-5
      u=alpha(1);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,9,3,xyz0);
//c     midpoint of side 5-c3'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      call xyzcart(u,v,r,xyz2,10,3,xyz0);
//c     midpoint of side c3'-c1'
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,11,3,xyz0);
//c     midpoint of side c1'-c2'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,12,3,xyz0);
//c     midpoint of side c2-2,2-c3,c3-c1,c1-c2
	for (k=1; k<=3; k++){
      xyz1(k,9)=xyz2(k,9);
      xyz1(k,10)=xyz2(k,10);
      xyz1(k,11)=xyz2(k,11);
      xyz1(k,12)=xyz2(k,12);
	}
      xyz1(3,9)=xyz2(3,9)-h;
      xyz1(3,10)=xyz2(3,10)-h;
      xyz1(3,11)=xyz2(3,11)-h;
      xyz1(3,12)=xyz2(3,12)-h;
//c     midpoint of side c2-c2',2-5,c3-c3',c1-c1'
	for (k=1; k<=3; k++){
      xyz1(k,21)=(xyz1(k,2)+xyz2(k,2))*FHALF;
      xyz1(k,22)=(xyzo(k,2)+xyzo(k,5))*FHALF;
      xyz1(k,23)=(xyz1(k,3)+xyz2(k,3))*FHALF;
      xyz1(k,24)=(xyz1(k,1)+xyz2(k,1))*FHALF;
	}
//c*    sub-zone 3
//c     midpoint of side c1'-c3'
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,13,3,xyz0);
//c     midpoint of side c3'-6
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      call xyzcart(u,v,r,xyz2,14,3,xyz0);
//c     midpoint of side 6-c4'
      u=alpha(2);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,15,3,xyz0);
//c     midpoint of side c4'-c1'
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,16,3,xyz0);
//c     midpoint of side c1-c3,c3-3,3-c4,c4-c1
	for (k=1; k<=3; k++){
      xyz1(k,13)=xyz2(k,13);
      xyz1(k,14)=xyz2(k,14);
      xyz1(k,15)=xyz2(k,15);
      xyz1(k,16)=xyz2(k,16);
	}
    xyz1(3,13)=xyz2(3,13)-h;
    xyz1(3,14)=xyz2(3,14)-h;
    xyz1(3,15)=xyz2(3,15)-h;
    xyz1(3,16)=xyz2(3,16)-h;
//c     midpoint of side c1-c1',c3-c3',3-6,c4-c4'
	for (k=1; k<=3; k++){
      xyz1(k,25)=(xyz1(k,1)+xyz2(k,1))*FHALF;
      xyz1(k,26)=(xyz1(k,3)+xyz2(k,3))*FHALF;
      xyz1(k,27)=(xyzo(k,3)+xyzo(k,6))*FHALF;
      xyz1(k,28)=(xyz1(k,4)+xyz2(k,4))*FHALF;
	}
}


static void
spshell(M3& xyz, const int izn, M1& xyz0, M1& alpha, M1& beta, 
		const FREAL& r, const FREAL& h, M2& xyzo, M2& xyz1, M2& xyz2)
{
	//dimension xyz(3,27,1),xyz0(3),beta(2),alpha(2)
    //dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
//c==	generate node coordinates
//c	node 1
    FREAL u=alpha(1);
    FREAL v=beta(1);
    FREAL rh=r-h;
      call xyzcart(u,v,rh,xyzo,1,3,xyz0);
//c	node 2
      u=alpha(1);
      v=beta(2);
      rh=r-h;
      call xyzcart(u,v,rh,xyzo,2,3,xyz0);
//c	node 6
      u=alpha(2);
      v=beta(2);
      rh=r-h;
      call xyzcart(u,v,rh,xyzo,3,3,xyz0);
//c	node 4
      u=alpha(1);
      v=beta(1);
      call xyzcart(u,v,r,xyzo,4,3,xyz0);
//c	node 5
      u=alpha(1);
      v=beta(2);
      call xyzcart(u,v,r,xyzo,5,3,xyz0);
//c	node 6
      u=alpha(2);
      v=beta(2);
      call xyzcart(u,v,r,xyzo,6,3,xyz0);
	int k, l;
	for (k=1; k<=3; k++)
		for (l=1; l<=6; l++)
			xyz(k,l,izn)=xyzo(k,l);
//c==   generate midpoint coordinates which are nodal coordinates 
//c==   each sub-zone
//c     midpoint on t. surface (c1)
      u=(alpha(2)+alpha(1))*FHALF;
      v=0.62*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,1,3,xyz0);
//c     midpoint of side 1-2 (c2)
      u=alpha(1);
      v=(beta(1)+beta(2))*FHALF;
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,2,3,xyz0);
//c     midpoint of side 2-3 (c3)
      u=(alpha(2)+alpha(1))*FHALF;
      v=beta(2);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,3,3,xyz0);
//c     midpoint of side 3-1 (c4)
      u=alpha(2);
      v=(beta(1)+beta(2))*FHALF;
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,4,3,xyz0);
//c     midpoint on t. surface (c1')
      u=(alpha(2)+alpha(1))*FHALF;
      v=0.62*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,1,3,xyz0);
//c     midpoint of side 4-5 (c2')
      u=alpha(1);
      v=(beta(1)+beta(2))*FHALF;
      rh=r;
      call xyzcart(u,v,rh,xyz2,2,3,xyz0);
//c     midpoint of side 5-6 (c3')
      u=(alpha(2)+alpha(1))*FHALF;
      v=beta(2);
      call xyzcart(u,v,r,xyz2,3,3,xyz0);
//c     midpoint of side 6-4 (c4')
      u=alpha(2);
      v=(beta(1)+beta(2))*FHALF;
      rh=r;
      call xyzcart(u,v,rh,xyz2,4,3,xyz0);
//c==   generate midpoint coordinates for sub-zones
//c*    sub-zone 1
//c     midpoint of side 1-c2
      u=alpha(1);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,5,3,xyz0);
//c     midpoint of side c2-c1
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,6,3,xyz0);
//c     midpoint of side c1-c4
      u=alpha(2)-(alpha(2)-alpha(1))*FQUARTER;
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,7,3,xyz0);
//c     midpoint of side c4-1
      u=alpha(2);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,8,3,xyz0);
//c     midpoint of side 4-c2'
      u=alpha(1);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,5,3,xyz0);
//c     midpoint of side c2'-c1'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      call xyzcart(u,v,r,xyz2,6,3,xyz0);
//c     midpoint of side c1'-c4'
      u=alpha(2)-(alpha(2)-alpha(1))*FQUARTER;
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,7,3,xyz0);
//c     midpoint of side c4'-4
      u=alpha(2);
      v=(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,8,3,xyz0);
//c     midpoint of side 1-4,c2-c2',c1-c1',c4-c4'
	for (k=1; k<=3; k++){
      xyz1(k,17)=(xyzo(k,1)+xyzo(k,4))*FHALF;
      xyz1(k,18)=(xyz1(k,2)+xyz2(k,2))*FHALF;
      xyz1(k,19)=(xyz1(k,1)+xyz2(k,1))*FHALF;
      xyz1(k,20)=(xyz1(k,4)+xyz2(k,4))*FHALF;
	}
//c*    sub-zone 2
//c     midpoint of side c2-2
      u=alpha(1);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,9,3,xyz0);
//c     midpoint of side 2-c3
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,10,3,xyz0);
//c     midpoint of side c3-c1
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,11,3,xyz0);
//c     midpoint of side c1-c2
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,12,3,xyz0);
//c     midpoint of side c2'-5
      u=alpha(1);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,9,3,xyz0);
//c     midpoint of side 5-c3'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      call xyzcart(u,v,r,xyz2,10,3,xyz0);
//c     midpoint of side c3'-c1'
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,11,3,xyz0);
//c     midpoint of side c1'-c2'
      u=(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,12,3,xyz0);
//c     midpoint of side c2-c2',2-5,c3-c3',c1-c1'
	for (k=1; k<=3; k++){
      xyz1(k,21)=(xyz1(k,2)+xyz2(k,2))*FHALF;
      xyz1(k,22)=(xyzo(k,2)+xyzo(k,5))*FHALF;
      xyz1(k,23)=(xyz1(k,3)+xyz2(k,3))*FHALF;
      xyz1(k,24)=(xyz1(k,1)+xyz2(k,1))*FHALF;
	}
//c*    sub-zone 3
//c     midpoint of side c1-c3
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,13,3,xyz0);
//c     midpoint of side c3-3
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,14,3,xyz0);
//c     midpoint of side 3-c4
      u=alpha(2);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,15,3,xyz0);
//c     midpoint of side c4-c1
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r-h;
      call xyzcart(u,v,rh,xyz1,16,3,xyz0);
//c     midpoint of side c1'-c3'
      u=alpha(2)-(alpha(2)-alpha(1))*FHALF;
      v=0.81*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,13,3,xyz0);
//c     midpoint of side c3'-6
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=beta(2);
      call xyzcart(u,v,r,xyz2,14,3,xyz0);
//c     midpoint of side 6-c4'
      u=alpha(2);
      v=3*(beta(2)-beta(1))*FQUARTER+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,15,3,xyz0);
//c     midpoint of side c4'-c1'
      u=3*(alpha(2)-alpha(1))*FQUARTER+alpha(1);
      v=0.551*(beta(2)-beta(1))+beta(1);
      rh=r;
      call xyzcart(u,v,rh,xyz2,16,3,xyz0);
//c     midpoint of side c1-c3,c3-3,3-c4,c4-c1
//c     midpoint of side c1-c1',c3-c3',3-6,c4-c4'
	for (k=1; k<=3; k++){
      xyz1(k,25)=(xyz1(k,1)+xyz2(k,1))*FHALF;
      xyz1(k,26)=(xyz1(k,3)+xyz2(k,3))*FHALF;
      xyz1(k,27)=(xyzo(k,3)+xyzo(k,6))*FHALF;
      xyz1(k,28)=(xyz1(k,4)+xyz2(k,4))*FHALF;
	}
}      


static void 
shemshzn(const int nsplitw, const int nsplith, const FREAL& r, const FREAL& h, 
	const FREAL&a1, const FREAL &a2, const FREAL &b1, const FREAL b2, 
	const int ishtype,
	MV3 &vertex)
{
	//dimension h(2),alpha(2),beta(2)
    //dimension ixstr(1),ixend(1),iystr(1),iyend(1), izstr(1),izend(1)
    //dimension nzsti(1),nzstj(1),nzstk(1)
    //dimension xyz(3,27,1)
    //dimension x(1),y(1),z(1)
    //dimension itype(1),dirc(3,3,1),xyz00(3,1),sca(3,1)
    //dimension xyzo(3,6),xyz1(3,28),xyz2(3,19)
    //dimension xyz0(3)
	const int NZONE=3;
	M1 xyz0(3), alpha(2), beta(2);  
	M3 xyz(3,27,4), dirc(3,3,1);
    M2 xyzo(3,6), xyz1(3,28), xyz2(3,19), sca(3,1);
	IA1 ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4);
	IA1 nzsti(NZONE), itype(NZONE);
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	itype(1)=itype(2)=itype(3)=1;
	alpha(1)=a1, alpha(2)=a2;
	beta(1)=b1, beta(2)=b2;

	int NX, NY;
	_decideSplitNumberCyl(r, h, nsplitw, NX, NY);
	const int NX3 = 3*NX;
	const int NX2 = NX*2-1;
	const int NY2 = NY*2-1;
	const int NZ = nsplith+1;
	vertex.reSize(NX3, NY, NZ), 
	nzsti(1) = 1;
	nzsti(2) = 1+NX;
	nzsti(3) = 1+2*NX;

	const int izn=1;
	  ixstr(1)=1, ixend(1)=NX2;
      iystr(izn)=ixstr(izn), iyend(izn)=ixend(izn);
      izstr(izn)=1, izend(izn)=NZ;
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
	if(ishtype==1)		//shell??
		call gshell(xyz,izn,xyz0,alpha,beta,r,h,xyzo,xyz1,xyz2);
    else if(ishtype==2)	//spherical shell
		call spshell(xyz,izn,xyz0,alpha,beta,r,h,xyzo,xyz1,xyz2);
	else
		assert(0);	//unknown paramter
//c	sub-zone 1
	int k;
	for (k=1; k<=3; k++){
      xyz(k,1,izn)=xyzo(k,1);
      xyz(k,2,izn)=xyz1(k,2);
      xyz(k,3,izn)=xyz1(k,1);
      xyz(k,4,izn)=xyz1(k,4);
      xyz(k,5,izn)=xyzo(k,4);
      xyz(k,6,izn)=xyz2(k,2);
      xyz(k,7,izn)=xyz2(k,1);
      xyz(k,8,izn)=xyz2(k,4);
      xyz(k,9,izn)=xyz1(k,5);
      xyz(k,10,izn)=xyz1(k,6);
      xyz(k,11,izn)=xyz1(k,7);
      xyz(k,12,izn)=xyz1(k,8);
      xyz(k,13,izn)=xyz1(k,17);
      xyz(k,14,izn)=xyz1(k,18);
      xyz(k,15,izn)=xyz1(k,19);
      xyz(k,16,izn)=xyz1(k,20);
      xyz(k,17,izn)=xyz2(k,5);
      xyz(k,18,izn)=xyz2(k,6);
      xyz(k,19,izn)=xyz2(k,7);
      xyz(k,20,izn)=xyz2(k,8);
	}
//c	sub-zone 2
	for (k=1; k<=3; k++){
      xyz(k,1,izn1)=xyz1(k,2);
      xyz(k,2,izn1)=xyzo(k,2);
      xyz(k,3,izn1)=xyz1(k,3);
      xyz(k,4,izn1)=xyz1(k,1);
      xyz(k,5,izn1)=xyz2(k,2);
      xyz(k,6,izn1)=xyzo(k,5);
      xyz(k,7,izn1)=xyz2(k,3);
      xyz(k,8,izn1)=xyz2(k,1);
      xyz(k,9,izn1)=xyz1(k,9);
      xyz(k,10,izn1)=xyz1(k,10);
      xyz(k,11,izn1)=xyz1(k,11);
      xyz(k,12,izn1)=xyz1(k,12);
      xyz(k,13,izn1)=xyz1(k,21);
      xyz(k,14,izn1)=xyz1(k,22);
      xyz(k,15,izn1)=xyz1(k,23);
      xyz(k,16,izn1)=xyz1(k,24);
      xyz(k,17,izn1)=xyz2(k,9);
      xyz(k,18,izn1)=xyz2(k,10);
      xyz(k,19,izn1)=xyz2(k,11);
      xyz(k,20,izn1)=xyz2(k,12);
	}
//c	sub-zone 3
	for (k=1; k<=3; k++){
      xyz(k,1,izn2)=xyz1(k,1);
      xyz(k,2,izn2)=xyz1(k,3);
      xyz(k,3,izn2)=xyzo(k,3);
      xyz(k,4,izn2)=xyz1(k,4);
      xyz(k,5,izn2)=xyz2(k,1);
      xyz(k,6,izn2)=xyz2(k,3);
      xyz(k,7,izn2)=xyzo(k,6);
      xyz(k,8,izn2)=xyz2(k,4);
      xyz(k,9,izn2)=xyz1(k,13);
      xyz(k,10,izn2)=xyz1(k,14);
      xyz(k,11,izn2)=xyz1(k,15);
      xyz(k,12,izn2)=xyz1(k,16);
      xyz(k,13,izn2)=xyz1(k,25);
      xyz(k,14,izn2)=xyz1(k,26);
      xyz(k,15,izn2)=xyz1(k,27);
      xyz(k,16,izn2)=xyz1(k,28);
      xyz(k,17,izn2)=xyz2(k,13);
      xyz(k,18,izn2)=xyz2(k,14);
      xyz(k,19,izn2)=xyz2(k,15);
      xyz(k,20,izn2)=xyz2(k,16);
	}

	//mesh all the 3 zones
	int iz=1, nx, ny, nz;
	MV3 mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, 1, nx, 1, ny, nz, nz); 
    validBoundaryVertices(r-h, mat, 1, nx, 1, ny, 1, 1); 

	iz=2;
	mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, 1, nx, 1, ny, nz, nz); 
    validBoundaryVertices(r-h, mat, 1, nx, 1, ny, 1, 1); 

	iz=3;
	mat=vertex(nzsti(iz));
	msh20zn(mat,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    nx = ixend(iz)-ixstr(iz)+1;
    ny = iyend(iz)-iystr(iz)+1;
    nz = izend(iz)-izstr(iz)+1;
    validBoundaryVertices(r, mat, 1, nx, 1, ny, nz, nz); 
    validBoundaryVertices(r-h, mat, 1, nx, 1, ny, 1, 1); 
}


void shereShell(const int nsplitw, const int nsplith, const FREAL &r, const FREAL &h,
	const FREAL &a1, const FREAL &a2, const FREAL &b1, const FREAL &b2, 
	MV3 &vertex)
{
	shemshzn(nsplitw, nsplith, r, h, a1, a2, b1, b2, 2, vertex);
}

/*
void cylinderShell(const int nsplitw, const int nsplitt, const int nsplith, 
	const FREAL &r, const FREAL &h, const FREAL &th,
	const FREAL &a1, const FREAL &a2, 
	MV3 &vertex)
{
	IA1 itype(4), ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4);
	M1 xyz0(3);
	M2 sca(3,1);
	M3 xyz(3,8,1), dirc(3,3,1);

	const int iz=1;
	ixstr(iz)=1, ixend(iz)=nsplitw+1;
	iystr(iz)=1, iyend(iz)=nsplitt+1;
	izstr(iz)=1, izend(iz)=nsplith+1;
    const int nx=ixend(iz)-ixstr(iz)+1;
    const int ny=iyend(iz)-iystr(iz)+1;
    const int nz=izend(iz)-izstr(iz)+1;
	xyz0(1)=xyz0(2)=xyz0(3)=0;
	vertex.reSize(nx, ny, nz);
	itype(1)=itype(2)=4;

	const int X=1;
	const int Y=2;
	const int Z=3;
	const FREAL rh = r-th;
	const double pa1=a1*PI/180;
	const double pa2=a2*PI/180;
	xyz(X,1,1)=r, xyz(Y,1,1)=pa1, xyz(Z,1,1)=0;
	xyz(X,2,1)=r, xyz(Y,2,1)=pa2, xyz(Z,2,1)=0;
	xyz(X,3,1)=rh, xyz(Y,3,1)=pa2, xyz(Z,3,1)=0;
	xyz(X,4,1)=rh, xyz(Y,4,1)=pa1, xyz(Z,4,1)=0;
	xyz(X,5,1)=r, xyz(Y,5,1)=pa1, xyz(Z,5,1)=h;
	xyz(X,6,1)=r, xyz(Y,6,1)=pa2, xyz(Z,6,1)=h;
	xyz(X,7,1)=rh, xyz(Y,7,1)=pa2, xyz(Z,7,1)=h;
	xyz(X,8,1)=rh, xyz(Y,8,1)=pa1, xyz(Z,8,1)=h;
    mesh8zn(vertex,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
}
*/


static void 
_setCylShellVertices(const FREAL a1, const FREAL a2, const FREAL r, const FREAL h, const FREAL th, M3 & xyz) 
{
    Vector3d v[21];
	const FREAL rh = r-th;
    const FREAL a12=(a1+a2)*0.5;

    xyzcart(a1, 0, r, 2, v[1]);
    xyzcart(a2, 0, r, 2, v[2]);
    xyzcart(a2, 0, rh, 2, v[3]);
    xyzcart(a1, 0, rh, 2, v[4]);

    v[5]=v[1], v[5].z=h;
    v[6]=v[2], v[6].z=h;
    v[7]=v[3], v[7].z=h;
    v[8]=v[4], v[8].z=h;

    xyzcart(a12, 0, r, 2, v[9]);
    v[10]=(v[2]+v[3])*0.5;
    xyzcart(a12, 0, rh, 2, v[11]);
    v[12]=(v[4]+v[1])*0.5;

    v[13]=(v[1]+v[5])*0.5;
    v[14]=(v[2]+v[6])*0.5;
    v[15]=(v[3]+v[7])*0.5;
    v[16]=(v[4]+v[8])*0.5;

    v[17]=v[9], v[17].z=h;
    v[18]=v[10], v[18].z=h;
    v[19]=v[11], v[19].z=h;
    v[20]=v[12], v[20].z=h;

    for (int i=1; i<=20; i++){
        xyz(1, i, 1) = v[i].x;
        xyz(2, i, 1) = v[i].y;
        xyz(3, i, 1) = v[i].z;
    }
}


void cylinderShell(const int nsplitw, const int nsplitt, const int nsplith, 
	const FREAL &r, const FREAL &h, const FREAL &th,
	const FREAL &a1, const FREAL &a2, 
	MV3 &vertex)
{
	IA1 itype(4), ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4);
	M3 xyz(3,20,1);

	const int iz=1;
	ixstr(iz)=1, ixend(iz)=nsplitw/2+1;
	iystr(iz)=1, iyend(iz)=nsplitt+1;
	izstr(iz)=1, izend(iz)=nsplith+1;
    const int nx=nsplitw/2+1;
    const int ny=iyend(iz)-iystr(iz)+1;
    const int nz=izend(iz)-izstr(iz)+1;
	vertex.reSize(nx*2, ny, nz);
	itype(1)=itype(2)=1;

    const FREAL a12=(a1+a2)*0.5;
    _setCylShellVertices(a1, a12, r, h, th, xyz); 
	msh20zn(vertex,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);
    _setCylShellVertices(a12, a2, r, h, th, xyz); 
    MV3 vertex2 = vertex(nx+1);
	msh20zn(vertex2,ixstr,ixend,iystr,iyend,izstr,izend,xyz,iz,itype);

}


//===================Tetrahedron Mesher======================================

void tetrahedronMesher(const int nsplitx, const int nsplity, const int nsplitz, const Vector3d v[4], MV3 &vertex)
{
	IA1 xstride(4), itype(4), ixstr(4), ixend(4), iystr(4), iyend(4), izstr(4), izend(4);
	M3 xyz(3,8,4);

	const int iz=1;
	const int NX=((nsplitx+1)/2)+1;
	const int NY=((nsplity+1)/2)+1;
	const int NZ=((nsplitz+1)/2)+1;
	const int NX2=NX*2-1;
	const int NY2=NY*2-1;
	const int NZ2=NZ*2-1;
	ixstr(iz)=1, ixend(iz)=NX2;
	iystr(iz)=1, iyend(iz)=NY2;
	izstr(iz)=1, izend(iz)=NZ2;
	itype(1)=itype(2)=itype(3)=itype(4)=1;
	xstride(1)=1;
	xstride(2)=1+NX;
	xstride(3)=1+2*NX;
	xstride(4)=1+3*NX;
	vertex.reSize(4*NX, NY, NZ);

	const int X=1;	const int Y=2; const int Z=3;
	xyz(X,1,1)=v[0].x, xyz(Y,1,1)=v[0].y, xyz(Z,1,1)=v[0].z;
	xyz(X,2,1)=v[1].x, xyz(Y,2,1)=v[1].y, xyz(Z,2,1)=v[1].z;
	xyz(X,3,1)=v[2].x, xyz(Y,3,1)=v[2].y, xyz(Z,3,1)=v[2].z;
	xyz(X,4,1)=v[3].x, xyz(Y,4,1)=v[3].y, xyz(Z,4,1)=v[3].z;
	mesh4zn(vertex, xstride, ixstr, ixend, iystr, iyend, izstr, izend, xyz, iz, itype);
}


