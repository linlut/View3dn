// Emacs should recognise this header as -*- C++ -*-

#ifndef CGHS_BLAS_H
#define CGHS_BLAS_H

// ============================================================================
//  CG nach Hestenes und Stiefel
//
//  siehe auch:
//  Ashby, Manteuffel, Saylor
//     A taxononmy for conjugate gradient methods
//     SIAM J Numer Anal 27, 1542-1568 (1990)
//
//  oder:
//		Willy D"orfler:
//		Orthogonale Fehlermethoden
//      Christian Badura, Mai 1998
// ============================================================================
/*
// ohne Vorkonditionierer
template< class MATRIX >
inline int
cghs( unsigned N, const MATRIX &A, const double *b, double *x, double eps );

template< class MATRIX >
inline int
cghs( unsigned N, const MATRIX &A, const double *b, double *x, double eps,
      bool detailed );


// mit Vorkonditionierer
template< class MATRIX, class PC_MATRIX >
inline int
cghs( unsigned N, const MATRIX &A, const PC_MATRIX &C,
      const double *b, double *x, double eps );

template< class MATRIX, class PC_MATRIX >
inline int
cghs( unsigned N, const MATRIX &A, const PC_MATRIX &C,
      const double *b, double *x, double eps, bool detailed );

*/
// ============================================================================
#include <memory.h>
#include <vector>
#include "cblas.h"

#define ZERODOTVAL (1e-180)

inline double *_getDoubleBuffer(const unsigned int N)
{
	static vector<double> buffer;
	const unsigned int nsize = buffer.size();
	if (N>nsize) buffer.resize(N);
	return &buffer[0];
}

//modified fast code of CG, using the Johnathan Jonathan Richard Shewchuk's tutorial
//http://www.cs.cmu.edu/~quake-papers/painless-conjugate-gradient.pdf
template< class MATRIX > 
inline int cghs(
	const unsigned int N, const unsigned int ITRMAX, const MATRIX &A, 
	const double *b, double *x, const double eps) 
{
	double *buf0 = _getDoubleBuffer(N*2);
    double *Ad = &buf0[0];
    double *r = &buf0[N];
    double *d = (double *)b;	//d buffer shares with b, this vialates the func. prototype def.
 
    //use zero as the initial guess for x
    dcopy(N,b,1,r,1);
    ///dcopy(N,b,1,d,1);	//this can be saved by allowing d shares b
    double r_dot_r = ddot(N,r,1,r,1);
	if (r_dot_r < ZERODOTVAL){
		memset(x, 0, sizeof(double)*N);
		return 0;
	}
	const double eps2 = r_dot_r*eps*eps;

    //alg. according to the tutorial of Jonathan R. Shewchuk
	unsigned int its;
    for (its=1; its<=ITRMAX; its++){
        mult(A, d, Ad);
        const double alpha = r_dot_r/ddot(N,d,1,Ad,1);
		if (its==1)
		    dcopy(N,d,1,x,1), dscal(N,alpha,x,1);
		else
			daxpy(N,alpha,d,1,x,1);
        daxpy(N,-alpha,Ad,1,r,1);
        const double r1_dot_r1 = ddot(N,r,1,r,1);
        if (r1_dot_r1<eps2) 
			break;
        const double beta = r1_dot_r1/r_dot_r;
        r_dot_r = r1_dot_r1;
        dscal(N,beta,d,1);
        daxpy(N,1,r,1,d,1);
    }
    return its;
}


template< class MATRIX > 
inline int cghs2(
	const unsigned int N, const unsigned int ITRMAX, const MATRIX &A, 
	const double *b, double *x, const double eps) 
{
    //memset(x, 0, sizeof(double)*N);
	double *buf0 = _getDoubleBuffer(N*3);
	double *g = &buf0[0];
	double *r = &buf0[N];
	double *p = &buf0[N+N];
	int its=0;
	double tau, sig, rho, gam;
	double err=eps*eps*ddot(N,b,1,b,1), err1;
  
	mult(A,x,g);
	daxpy(N,-1.,b,1,g,1);
	dscal(N,-1.,g,1);
	dcopy(N,g,1,r,1);
	while ((its<ITRMAX)){
		mult(A,r,p);
		rho=ddot(N,p,1,p,1);
		sig=ddot(N,r,1,p,1);
		tau=ddot(N,g,1,r,1);
		const double t=tau/sig;
		daxpy(N,t,r,1,x,1);
		daxpy(N,-t,p,1,g,1);
		gam=(t*t*rho-tau)/tau;
		dscal(N,gam,r,1);
		daxpy(N,1.,g,1,r,1);
		//if ( detailed ){
		//	cout<<"cghs "<<its<<"\t"<<dnrm2(N,g,1)<<endl;
		//}
		++its;
		err1 = ddot(N,g,1,g,1);
		if (err1<err) break;
	}
	return its;
}


//========================================================================
template< class MATRIX, class PC_MATRIX > 
inline int cghs(
	const unsigned int N,			//number of vertices
	const unsigned int ITRMAX,		//max iteration number
	const MATRIX &A,				//Ax = b
	const PC_MATRIX &C,				//pre-condition diagonal matrix
	const double *b,				//
	double *x,						//result x
	const double eps)				//error tolerance
{
	unsigned int its;
	double *buf0 = _getDoubleBuffer(N*2);
	double *r = (double*)b; 
	double *d = &buf0[0];
	double *h = &buf0[N];
	double *Ad = h;

	const double eps2 = eps*eps;
	const double r_dot_r=ddot(N,b,1,b,1);
	if (r_dot_r<ZERODOTVAL){
		memset(x, 0, sizeof(double)*N);
		return 0;
	}
	const double err=r_dot_r*eps2;
	precond_mult(C, r, N, d);
	double rh=ddot(N,r,1,d,1);
	const double err2=rh*eps2;

	for (its=1; its<=ITRMAX; its++) {
		mult(A,d,Ad);
		const double alpha=rh/ddot(N,d,1,Ad,1);
		if (its==1)
		    dcopy(N,d,1,x,1), dscal(N,alpha,x,1);
		else
			daxpy(N,+alpha,d,1,x,1);
		daxpy(N,-alpha,Ad,1,r,1);
		if (ddot(N,r,1,r,1)<err) break;
		precond_mult(C, r, N, h);
		const double rh1=ddot(N,r,1,h,1);
		if (rh1<err2) break;
		const double beta=rh1/rh;;
		rh=rh1;
		dscal(N,beta,d,1);
		daxpy(N,1.0,h,1,d,1);
	}
	return its;
}


//Original code for PCG
template< class MATRIX, class PC_MATRIX > 
inline int cghs2(
	unsigned int N, const unsigned int ITRMAX, 
	const MATRIX &A, const PC_MATRIX &C,
    const double *b, double *x, double eps) 
{
	double *buf0 = _getDoubleBuffer(N*3);
	double *r = &buf0[0];
	double *d = &buf0[N];
	double *h = &buf0[N+N];
	double *Ad = h;
	double rh, alpha, beta, err=eps*eps*ddot(N,b,1,b,1);
	unsigned int its;

	mult(A,x,r);
	daxpy(N,-1.,b,1,r,1);
	mult(C,r,d);
	dcopy(N,d,1,h,1);
	rh=ddot(N,r,1,h,1);
	for (its=0; its<ITRMAX; its++) {
		mult(A,d,Ad);
		alpha=rh/ddot(N,d,1,Ad,1);
		daxpy(N,-alpha,d,1,x,1);
		daxpy(N,-alpha,Ad,1,r,1);
		mult(C,r,h);
		beta=1.0/rh; rh=ddot(N,r,1,h,1); beta*=rh;
		dscal(N,beta,d,1);
		daxpy(N,1.,h,1,d,1);
		if (ddot(N,r,1,r,1)>err) break;
	}
	return its;
}

// ============================================================================


#endif // CGHS_BLAS_H
