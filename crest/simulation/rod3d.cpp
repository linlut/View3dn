#include <vectorall.h>
#include <matrix3x3.h>

#define Tensor3x3 double3x3


/* File: S_TENS_INV.f
      SUBROUTINE S_TENS_INV(T,G)  
C***  CALCULATES INVERSE OF SYMMETRIC TENSOR T               
c     T(1) = T(1,1)
c     T(2) = T(2,2)
c     T(3) = T(3,3)
c     T(4) = T(2,3)
c     T(5) = T(1,3)
c     T(6) = T(1,2)
c
c     T_11  T_12  T_13
c     T_21  T_22  T_23
c     T_31  T_32  T_33
c
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION T(*),G(*)    
C     A_{1,1}=T_{2,2}*T_{3,3}-T_{2,3}*T_{3,2}
      A11 =T(2)*T(3)-T(4)*T(4) 
C     A_{2,2}=T_{1,1}*T_{3,3}-T_{1,3}*T_{3,1}
      A22 =T(1)*T(3)-T(5)*T(5) 
C     A_{3,3}=T_{1,1}*T_{2,2}-T_{1,2}*T_{2,1}
      A33 =T(1)*T(2)-T(6)*T(6) 
C     A_{1,2}=T_{2,1}*T_{3,3}-T_{3,1}*T_{2,3}
      A12 =T(6)*T(3)-T(5)*T(4) 
C     A_{1,3}=T_{2,1}*T_{3,2}-T_{3,1}*T_{2,2}
      A13 =T(6)*T(4)-T(5)*T(2) 
C     A_{2,3}=T_{1,1}*T_{3,2}-T_{3,1}*T_{1,2}
      A23 =T(1)*T(4)-T(5)*T(6) 
c
      DJ = S_TENSOR_I3(T)  
c
      G(1)=A11/DJ
      G(2)=A22/DJ
      G(3)=A33/DJ
      G(4)=-A23/DJ
      G(5)=A13/DJ
      G(6)=-A12/DJ
c                                         
      RETURN                                                            
      END                                                               
*/

inline void inverseTensor(const Tensor3x3& M, Tensor3x3 & _G)  
{
	const double *T = M.x - 1;
	double A11 = T[2]*T[3]-T[4]*T[4];
	double A22 = T[1]*T[3]-T[5]*T[5]; 
	double A33 = T[1]*T[2]-T[6]*T[6];
	double A12 = T[6]*T[3]-T[5]*T[4]; 
    double A13 = T[6]*T[4]-T[5]*T[2];
    double A23 = T[1]*T[4]-T[5]*T[6]; 
    const double DJ = M.Det();
	const double DJ1 = 1.0/DJ;

	double *G = _G.x - 1;
    G[1]=A11*DJ1;
    G[2]=A22*DJ1;
    G[3]=A33*DJ1;
    G[4]=-A23*DJ1;
    G[5]=A13*DJ1;
    G[6]=-A12*DJ1;
}


/*
 MOD_GEOM_DIREC.f
 
          SUBROUTINE  ROD_F_TENS(ELCOD,DLCOD,ELDIS,XIJ,TXIJ,DIJ,FIJ,
     .                      TR2_ROD,R2_ROD)
             INCLUDE 'cia3d_precision.h'
             DIMENSION DLCOD(3,*),ELCOD(3,*),ELDIS(3,*)
             DIMENSION  XIJ(3),TXIJ(3),DIJ(3),FIJ(3,3)
             NDIME=3
             DO I=1,NDIME
               XIJ(I)=ELCOD(I,1)-ELCOD(I,2)
               TXIJ(I)=DLCOD(I,1)-DLCOD(I,2)
               DIJ(I)=ELDIS(I,1)-ELDIS(I,2)
             ENDDO
c------------------------------------------
c             print *,' XIJ = ',XIJ
c             print *,' TXIJ = ',TXIJ
c------------------------------------------
             R_ROD=V3D_NOR_2(XIJ)
             TR_ROD=V3D_NOR_2(TXIJ)
             R2_ROD=R_ROD*R_ROD
             TR2_ROD=TR_ROD*TR_ROD
             DO I=1,NDIME
               DO J=1,NDIME
               FIJ(I,j)=DELTA(I,J)+DIJ(I)*XIJ(J)/R2_ROD
               ENDDO
             ENDDO
             RETURN

          END SUBROUTINE ROD_F_TENS
*/

//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: 
//		TXJ:
//		FIJ: incremental deformation gradient
//		R2_ROD: length^2 at t_i
//----------------------------------------------------------
inline void computeARCDeformationGradient( 
		const Vector3d& XI, const Vector3d& TXI, 
		const Vector3d& XJ, const Vector3d& TXJ, 
		Tensor3x3& FIJ,
		Vector3d& XIJ,
		double& R2_ROD)
{
	XIJ = XI - XJ;
	const Vector3d TXIJ = TXI - TXJ;
    const Vector3d DIJ =  TXIJ - XIJ;	
	R2_ROD = Magnitude2(XIJ);

	//a tensor product operation
	const double k = 1.0/R2_ROD;
	XIJ*=k;
	vectorTensorProduct(DIJ, XIJ, FIJ);

	//add the identity tensor
	FIJ[0]+=1;
	FIJ[4]+=1;
	FIJ[8]+=1;
}

//----------------------------------------------------------
//Compute the deriv of the ARC deformation gradient with respect to the 
//delta of incremental displacement DIJ
//Notice that XIJ is already divided by R2_ROD
//----------------------------------------------------------
inline void computeARCDeformationGradientDeriv(const Vector3d& XIJ, Tensor3x3 deriv[3])
{
	Vector3d *p;
	p = (Vector3d*)(&deriv[0].x[0]);
	*p = XIJ;
	p = (Vector3d*)(&deriv[1].x[3]);
	*p = XIJ;
	p = (Vector3d*)(&deriv[2].x[6]);
	*p = XIJ;
}

inline void computeGreenStrainTensor(const Tensor3x3 &F, Tensor3x3 &strain)
{
	strain = FtF(F);
	strain.x[0] -= 1;
	strain.x[4] -= 1;
	strain.x[8] -= 1;
	strain *= 0.5;
}

inline void computeForceTerm0(
	const Tensor3x3& F_t, const Tensor3x3& F_tn, const Tensor3x3 Deriv_arc[3], Tensor3x3 F[3])
{
	Tensor3x3 G = F_t;
	G.Transpose();

	Tensor3x3 T0= multRow0(G, Deriv_arc[0]);
	Tensor3x3 T1= multRow1(G, Deriv_arc[1]);
	Tensor3x3 T2= multRow2(G, Deriv_arc[2]);
	F[0] = T0 * F_tn;
	F[1] = T1 * F_tn;
	F[2] = T2 * F_tn;

	//finally, add the transpose of itself
	F[0].AddTranspose();	
	F[1].AddTranspose();	
	F[2].AddTranspose();	
}

/*
      SUBROUTINE TO_LAME(YOUNG,POISS,DL,DM)     
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
c
C=    get Lame conefficient lamda and nu from E and NU       
      DL=YOUNG*POISS/(1.D0+POISS)/(1.D0-2.D0*POISS)
      DM=YOUNG/(1.D0+POISS)/2.D0
      RETURN                                                            
      END           
*/
inline void YoungsModulus2Lame(const double YOUNG, const double POISS, double &Lambda, double &Miu)
{
	const double POISS1 = 1.00/(1.0+POISS);
	//lambda = vE/(1+v)/(1-2v)
	Lambda = YOUNG * POISS *POISS1 /(1.0- POISS - POISS);
	//miu = E/2/(1+v)
	Miu = YOUNG * POISS1 * 0.5;
}

inline void IsotropicMaterialStress(const double& Lambda, const double& Miu, const Tensor3x3& strain, Tensor3x3& stress)
{
	//copy some strains
	const double e11 = strain.x[0];
	const double e22 = strain.x[4];
	const double e33 = strain.x[8];
	const double M2 = Miu+Miu;
	const double LE123 = Lambda*(e11+e22+e33); 
	const double m11= LE123 + M2*e11;
	const double m22= LE123 + M2*e22;
	const double m33= LE123 + M2*e33;

	stress.x[0] = m11;
	stress.x[4] = m22;
	stress.x[8] = m33;
	stress.x[1] = stress.x[3] = strain.x[1]*M2;
	stress.x[2] = stress.x[6] = strain.x[2]*M2;
	stress.x[5] = stress.x[7] = strain.x[5]*M2;
}


Vector3d clacPos(const double RI, const double ang0)
{
	Vector3d XI;
	XI.x = RI*cos(ang0); XI.y = 0; XI.z = RI*sin(ang0);
	return XI;
}
static int test1(void)
{
	//
	double E=1e3;
	double POISSON = 0.3;
	double Lambda, Miu;
	Vector3d XI(0,0,0), TXI(0,0,0);
	Vector3d XJ(0,0,1);
	//Vector3d TXJ(0,0,1);
	Vector3d TXJ(0.017452406437283512819418978516316, 0.0, 0.99984769515639123915701155881391);
	Vector3d XIJ, XJI;
	Tensor3x3 FIJ, FJI, strain, stress;
	double R2_ROD, R3;
	Tensor3x3 deriv[3], F[3], FF;
	Tensor3x3 F_tn, F_t;
	F_tn.IdentityMatrix();
	const double RI = 1;
	const double RJ = 2;
	for (int i=0; i<30; i++){
		double ang0 = (0.0+i)*PI/180.0;
		double ang1 = (0.0+i+1.0)*PI/180.0;
		XI=clacPos(RI, ang0);
		TXI=clacPos(RI, ang1);
		XJ=clacPos(RJ, ang0);
		TXJ=clacPos(RJ, ang1);
		computeARCDeformationGradient(XI, TXI, XJ, TXJ, FIJ, XIJ, R2_ROD);
		computeARCDeformationGradientDeriv(XIJ, deriv);

		computeARCDeformationGradient(XJ, TXJ, XI, TXI, FJI, XJI, R3);
		FF = FIJ*FJI;
		//get the total deformation gradient
		F_t = FIJ* F_tn;

		computeForceTerm0(F_t, F_tn, deriv, F);
		computeGreenStrainTensor(F_t, strain);
		YoungsModulus2Lame(E, POISSON, Lambda, Miu);

		IsotropicMaterialStress(Lambda, Miu, strain, stress);
		F_tn = F_t;
	}

	//===========================================
	int mylist[10];
	{
		float MAXDOUBLE = 1e30f;
		int N3=3;
		int nsize = 10;
		float dist[10]={1, 3, 90, 7, 8, 4, 6, 2, 88, 63};
		int vertexid[10]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		
		for (int i=0; i<N3; i++){
			double MINDIST = dist[i];
			int id = vertexid[i], ipos=i;
			for (int j=0; j<nsize; j++){
				if (MINDIST>dist[j]){
					MINDIST = dist[j];
					id = vertexid[j];
					ipos = j;
				}
			}
			mylist[i]=id;
			dist[ipos]= MAXDOUBLE;
		}
	}
	//===========================================
	return 1;
}

static int rrr=test1();
