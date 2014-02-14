//FILE: etkQuartic.cu

#ifndef etkQuadric_hpp
#define etkQuadric_hpp
struct QuarticSolverResult
{
	int num;			//number of roots
	QFLOAT roots[4];	//the four roots
};
#endif

//=====================Local definitions=====================
#define doub0 0.0
#define doub1 1.0
#define doub2 2.0
#define doub3 3.0
#define doub4 4.0
#define doub6 6.0
#define doub12 12
#define doub24 24
#define inv2 0.5
#define inv3 0.3333333333333333333333333
#define inv4 0.250

#define doubmax 1e36	//Approx square root of max QFLOAT number
#define doubtol 1e-6	//Tolerance for deciding the quadirc function or bi-quadirc function
						//This value seems to be very important for single precision computation
						//For double precision computation, it is not sensitive to this value
//=====================Local definitions=====================
//Define sqrt(3.0);
#define rt3 1.7320508075688772935274463415059


// ----------------------------------------------------------------------------
// find cos(acos(x)/3)
// Don Herbison-Evans   16/7/81
// called by cubic .

__device__ inline QFLOAT acos3(const QFLOAT x)
{
   const QFLOAT value= cos(acos(x)*inv3);
   return value;
}

// ----------------------------------------------------------------------------
// find cube root of x.
// Don Herbison-Evans   30/1/89
// called by cubic .

__device__ inline QFLOAT curoot(const QFLOAT x)
{
   QFLOAT value;
   QFLOAT absx;
   int neg;

   neg = 0;
   absx = x;
   if (x < doub0){
      absx = -x;
      neg = 1;
   }
   if (absx != doub0) 
	   value = exp( log(absx)*inv3 );
   else 
	   value = doub0;
   if (neg == 1) 
	   value = -value;
   return(value);
}

// ----------------------------------------------------------------------------
// find the errors
// called by quartic.
/*
__device__ void errors(const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d, QFLOAT rts[4], QFLOAT rterr[4], int nrts)
{
   int k;
   QFLOAT deriv, test;

   if (nrts > 0){
      for (k = 0 ; k < nrts ; ++ k){
         test = (((rts[k]+a)*rts[k]+b)*rts[k]+c)*rts[k]+d ;
         if (test == doub0) 
			 rterr[k] = doub0;
         else{
            deriv =((doub4*rts[k]+doub3*a)*rts[k]+doub2*b)*rts[k]+c ;
            if (deriv != doub0)
               rterr[k] = fabs(test/deriv);
            else{
               deriv = (doub12*rts[k]+doub6*a)*rts[k]+doub2*b ;
               if (deriv != doub0)
                   rterr[k] = sqrt(fabs(test/deriv)) ;
               else{
                  deriv = doub24*rts[k]+doub6*a ;
                  if (deriv != doub0)
                     rterr[k] = curoot(fabs(test/deriv));
                  else
                     rterr[k] = sqrt(sqrt(fabs(test)/doub24));
               }
            }
         }
         if (rts[k] != doub0) 
			 rterr[k] /= rts[k];
         if (rterr[k] < doub0) 
			 rterr[k] = -rterr[k];
      }
   }
}
*/

// ----------------------------------------------------------------------------
// solve the quadratic equation -
//  x**2+b*x+c = 0
// called by  quartic, descartes, ferrari, neumark.
__device__ inline QuarticSolverResult 
quadratic(const QFLOAT b, const QFLOAT c, const QFLOAT dis)
{
	QuarticSolverResult result;

	if (dis > doub0){
		QFLOAT rts[2];
		QFLOAT rtdis = sqrt(dis) ;
		if (b > doub0) 
			rts[0] = (-b - rtdis)*inv2 ;
		else 
			rts[0] = (-b + rtdis)*inv2 ;
		if (rts[0] == doub0) 
			rts[1] = -b ;
		else 
			rts[1] = c/rts[0] ;
		result.num = 2 ;
		result.roots[0] = rts[0];
		result.roots[1] = rts[1];
	}
	else{
		result.num = 0;
		result.roots[0] = result.roots[1] = 0;
	}

	return result;
}

/*
__device__ inline int 
quadratic(const QFLOAT b, const QFLOAT c, QFLOAT rts[4], const QFLOAT dis)
{
   int nquad=0;
   QFLOAT rtdis ;

   if (dis > doub0){
      nquad = 2 ;
      rtdis = sqrt(dis) ;
      if (b > doub0) 
		  rts[0] = ( -b - rtdis)*inv2 ;
      else 
		  rts[0] = ( -b + rtdis)*inv2 ;
      if (rts[0] == doub0) 
		  rts[1] =  -b ;
      else 
		  rts[1] = c/rts[0] ;
   }
   else{
      nquad = 0;
      rts[0] = 0 ;
      rts[1] = 0 ;
   }
   return(nquad);
}
*/

// ----------------------------------------------------------------------------
__device__ inline QFLOAT cubic(const QFLOAT p, const QFLOAT q, const QFLOAT r)
/* ====================================================
   find the lowest real root of the cubic - 
       x**3 + p*x**2 + q*x + r = 0 
   input parameters - 
     p,q,r - coeffs of cubic equation. 
   output- 
     cubic - a real root. 
   global constants -
     rt3 - sqrt(3) 
     inv3 - 1/3 
     doubmax - square root of largest number held by machine 
   method - 
     see D.E. Littlewood, "A University Algebra" pp.173 - 6 
     initial version: Charles Prineas   April 1981 
   called by  neumark, descartes, ferrari.
   calls  acos3 
========================================================*/
{
	QFLOAT po3,po3sq,qo3;
	QFLOAT uo3,u2o3,uo3sq4,uo3cu4 ;
	QFLOAT v,vsq,wsq ;
	QFLOAT m,mcube,n;
	QFLOAT muo3,s,scube,t,cosk,sinsqk ;
	QFLOAT root;
   
	m = doub0;
	if ((p > doubmax) || (p < -doubmax)) 
	   root = -p;
	else if ((q > doubmax) || (q < -doubmax)){
		if (q > doub0) 
			root = -r/q;
		else if (q < 0) 
			root = -sqrt(-q);
		else 
			root = 0;
	}
	else if ((r > doubmax)|| (r <  -doubmax)) 
		root =  -curoot(r) ;
	else{
		po3 = p*inv3 ;
		po3sq = po3*po3 ;
		if (po3sq > doubmax) 
			root =  -p ;
		else{
			//po3cu = po3*po3sq ;
			v = r + po3*(po3sq + po3sq - q) ;
			if ((v > doubmax) || (v < -doubmax)) 
				root = -p ;
			else{
				vsq = v*v ;
				qo3 = q*inv3 ;
				uo3 = qo3 - po3sq ;
				u2o3 = uo3 + uo3 ;
				if ((u2o3 > doubmax) || (u2o3 < -doubmax)){
				if (p == doub0){
					if (q > doub0) 
						root =  -r/q ;
					else if (q < doub0) 
						root =  -sqrt(-q);
					else root =  doub0 ;
				}
				else 
					root =  -q/p ;
            }
            uo3sq4 = u2o3*u2o3 ;
            if (uo3sq4 > doubmax){
				if (p == doub0){
					if (q > doub0) 
						root = -r/q ;
					else if (q < doub0) 
						root = -sqrt(-q) ;
					else 
						root = doub0;
				}
				else 
					root = -q/p ;
            }
            uo3cu4 = uo3sq4*uo3 ;
            wsq = uo3cu4 + vsq ;
            if (wsq > doub0){
			//cubic has one real root 
				if (v <= doub0) mcube = ( -v + sqrt(wsq))*inv2 ;
				if (v  > doub0) mcube = ( -v - sqrt(wsq))*inv2 ;
				m = curoot(mcube) ;
				if (m != doub0) 
					n = -uo3/m ;
				else 
					n = doub0;
				root = m + n - po3 ;
			}
            else{
				//cubic has three real roots 
				if (uo3 < doub0){
					muo3 = -uo3;
					if (muo3 > doub0) 
						s = sqrt(muo3);
					else 
						s = doub0;
					scube = s*muo3;
					if (scube == doub0){
						//exit(1);
					}
					t =  -v/(scube+scube) ;
					cosk = acos3(t) ;
					if (po3 < doub0)
						root = (s+s)*cosk - po3;
					else{
						sinsqk = doub1 - cosk*cosk ;
						if (sinsqk > doub0)
							root = s*( -cosk - rt3*sqrt(sinsqk)) - po3 ;
						else 
							root = -s*cosk - po3;
					}
				}
				else
					//cubic has multiple root -  
					root = curoot(v) - po3 ;
				}
			}
		}
   }
   return root;
}

// ----------------------------------------------------------------------------
/*
__device__ inline int 
descartes(const QFLOAT& a, const QFLOAT& b, const QFLOAT& c, const QFLOAT& d, QFLOAT rts[4])
//   Solve quartic equation using
//   Descartes-Euler-Cardano algorithm
//   Strong, T. "Elemementary and Higher Algebra"
//      Pratt and Oakley, p. 469 (1859)
//     29 Jun 1994  Don Herbison-Evans
{
   int nrts;
   int r1,r2;
   QFLOAT v1[4],v2[4];
   QFLOAT y;
   QFLOAT p,q,r;
   QFLOAT A,B,C;
   QFLOAT m,n1,n2;
   QFLOAT asq;
   QFLOAT Ainv4;
   QFLOAT Binvm;
   const QFLOAT d3o8 = 3.0/8.0;
   const QFLOAT inv8 = doub1/8.0;
   const QFLOAT inv16 = doub1/16.0;
   const QFLOAT d3o256 = 3.0/256.0;

   asq = a*a;
   A = b - asq*d3o8;
   B = c + a*(asq*inv8 - b*inv2);
   C = d + asq*(b*inv16 - asq*d3o256) - a*c*inv4;

   p = doub2*A;
   q = A*A - doub4*C;
   r = -B*B;

   y = cubic(p,q,r) ;
   if (y <= doub0) 
      nrts = 0;
   else{
      m = sqrt(y);
      Ainv4 = a*inv4;
      Binvm = B/m;
      n1 = (y + A + Binvm)*inv2;
      n2 = (y + A - Binvm)*inv2;
      r1 = quadratic(-m, n1, v1, y-doub4*n1);
      r2 = quadratic( m, n2, v2, y-doub4*n2);
      rts[0] = v1[0]-Ainv4;
      rts[1] = v1[1]-Ainv4;
      rts[r1] = v2[0]-Ainv4;
      rts[r1+1] = v2[1]-Ainv4;
      nrts = r1+r2;
   } 
   return(nrts);
}
*/

// ----------------------------------------------------------------------------
/*
__device__ inline int 
ferrari(const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d, QFLOAT rts[4])
//==================================================== 
// solve the quartic equation - 
// x**4 + a*x**3 + b*x**2 + c*x + d = 0 
// called by quartic
// calls cubic, quadratic.
//
// input - 
//		a,b,c,e - coeffs of equation. 
// output - 
//		nquar - number of real roots. 
//		rts - array of root values. 
// method :  Ferrari - Lagrange
//    Theory of Equations, H.W. Turnbull p. 140 (1947)
// calls  cubic, quadratic 
======================================================
{
	int nquar,n1,n2 ;
	QFLOAT v1[4],v2[4] ;
	QFLOAT e,f,ef ;
	QFLOAT g,gg,h,hh;
	const QFLOAT ainv2 = a*inv2;
	const QFLOAT asqinv4 = ainv2*ainv2;
	const QFLOAT d4 = d*doub4;
	const QFLOAT p = b ;
	const QFLOAT q = a*c-d4 ;
	const QFLOAT r = (asqinv4 - b)*d4 + c*c ;
	const QFLOAT y = cubic(p,q,r) ;
	const QFLOAT yinv2 = y*inv2;
	const QFLOAT esq = asqinv4 - b - y;
	const QFLOAT fsq = yinv2*yinv2 - d;

	if ((esq < doub0) && (fsq < doub0)) 
		nquar = 0;
	else{
		ef = -(inv4*a*y + inv2*c);
		if ( ((a > doub0)&&(y > doub0)&&(c > doub0))
			|| ((a > doub0)&&(y < doub0)&&(c < doub0))
			|| ((a < doub0)&&(y > doub0)&&(c < doub0))
			|| ((a < doub0)&&(y < doub0)&&(c > doub0))
			||  (a == doub0)||(y == doub0)||(c == doub0)){
			// use ef - 
            if ((b < doub0)&&(y < doub0)){
               e = sqrt(esq);
               f = ef/e;
            }
            else if (d < doub0){
               f = sqrt(fsq);
               e = ef/f;
            }
            else{
               if (esq > doub0) e = sqrt(esq); else e = doub0;
               if (fsq > doub0) f = sqrt(fsq); else f = doub0;
               if (ef < doub0) f = -f;
            }
        }
		else{// use esq and fsq - 
            if (esq > doub0) e = sqrt(esq); else e = doub0;
            if (fsq > doub0) f = sqrt(fsq); else f = doub0;
            if (ef < doub0) f = -f;
		}
		/// note that e >= doub0 
        g = ainv2 - e;
        gg = ainv2 + e;
        if (((b > doub0)&&(y > doub0)) || ((b < doub0)&&(y < doub0))){
            if ((a > doub0) && (e > doub0) || (a < doub0) && (e < doub0) ) 
				g = (b + y)/gg;
            else if ((a > doub0) && (e < doub0) || (a < doub0) && (e > doub0) ) 
				gg = (b + y)/g;
        }
        h = -yinv2 - f;
        hh = -yinv2 + f;
		if ( ((f > doub0)&&(y < doub0)) || ((f < doub0)&&(y > doub0)) ) 
			h = d/hh;
        else if ( ((f < doub0)&&(y < doub0)) || ((f > doub0)&&(y > doub0)) ) 
			hh = d/h;
        n1 = quadratic(gg, hh, v1, gg*gg-doub4*hh) ;
        n2 = quadratic(g, h, v2, g*g-doub4*h) ;
        nquar = n1+n2 ;
        rts[0] = v1[0] ;
        rts[1] = v1[1] ;
        rts[n1+0] = v2[0] ;
        rts[n1+1] = v2[1] ;
	}
	return nquar;
} 
*/

__device__ inline QuarticSolverResult 
copyResults(QuarticSolverResult result1, QuarticSolverResult result2)
{
	QuarticSolverResult result={0,{0,0,0,0}};
	const int n1 = result1.num;
	result.num = n1 + result2.num;
	result.roots[0] = result1.roots[0];
	result.roots[1] = result1.roots[1];
	result.roots[n1] = result2.roots[0];
	result.roots[n1+1] = result2.roots[1];
	return result;
}


__device__ inline QuarticSolverResult 
ferrari(const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d)
{
	QuarticSolverResult result={0,{0,0,0,0}};
	QFLOAT e,f,ef ;
	const QFLOAT ainv2 = a*inv2;
	const QFLOAT asqinv4 = ainv2*ainv2;
	const QFLOAT d4 = d*doub4;
	const QFLOAT p = b ;
	const QFLOAT q = a*c-d4 ;
	const QFLOAT r = (asqinv4 - b)*d4 + c*c ;
	const QFLOAT y = cubic(p,q,r) ;
	const QFLOAT yinv2 = y*inv2;
	const QFLOAT esq = asqinv4 - b - y;
	const QFLOAT fsq = yinv2*yinv2 - d;

	if ((esq < doub0) && (fsq < doub0)) 
		result.num = 0;
	else{
		ef = -(inv4*a*y + inv2*c);
		if ( ((a > doub0)&&(y > doub0)&&(c > doub0)) || ((a > doub0)&&(y < doub0)&&(c < doub0))
			 || ((a < doub0)&&(y > doub0)&&(c < doub0)) || ((a < doub0)&&(y < doub0)&&(c > doub0))
			 || (a == doub0) || (y == doub0) || (c == doub0) ){
			// use ef - 
            if ((b < doub0)&&(y < doub0)){
               e = sqrt(esq);
               f = ef/e;
            }
            else if (d < doub0){
               f = sqrt(fsq);
               e = ef/f;
            }
            else{
               if (esq > doub0) e = sqrt(esq); else e = doub0;
               if (fsq > doub0) f = sqrt(fsq); else f = doub0;
               if (ef < doub0) f = -f;
            }
        }
		else{// use esq and fsq -
            if (esq > doub0) e = sqrt(esq); else e = doub0;
            if (fsq > doub0) f = sqrt(fsq); else f = doub0;
            if (ef < doub0) f = -f;
		}
		// note that e >= doub0 
        QFLOAT g = ainv2 - e;
        QFLOAT gg = ainv2 + e;
        if (((b > doub0)&&(y > doub0)) || ((b < doub0)&&(y < doub0))){
            if ((a > doub0) && (e > doub0) || (a < doub0) && (e < doub0) ) 
				g = (b + y)/gg;
            else if ((a > doub0) && (e < doub0) || (a < doub0) && (e > doub0) ) 
				gg = (b + y)/g;
        }
        QFLOAT h = -yinv2 - f;
        QFLOAT hh = -yinv2 + f;
		if ( ((f > doub0)&&(y < doub0)) || ((f < doub0)&&(y > doub0)) ) 
			h = d/hh;
        else if ( ((f < doub0)&&(y < doub0)) || ((f > doub0)&&(y > doub0)) ) 
			hh = d/h;
		//=======================================
        QuarticSolverResult result1 = quadratic(gg, hh, gg*gg-doub4*hh) ;
        QuarticSolverResult result2 = quadratic(g, h, g*g-doub4*h);
		result = copyResults(result1, result2);
	}

	return result;
} // ferrari 


__device__ inline QuarticSolverResult 
neumark(const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d)
   //solve the quartic equation - 
   //x**4 + a*x**3 + b*x**2 + c*x + d = 0 
   //called by quartic
   //calls cubic, quadratic.
   //input parameters - 
   //	a,b,c,e - coeffs of equation. 
   //output parameters - 
   //	nquar - number of real roots. 
   //	rts - array of root values. 
   //method -  S. Neumark 
   //	Solution of Cubic and Quartic Equations - Pergamon 1965 
{
   QFLOAT g,gg,h,hh,gdis,gdisrt,hdis,hdisrt,g1,g2,h1,h2 ;
   QFLOAT bmy,gerr,herr,y4,bmysq ;
   QFLOAT hmax,gmax ;
   QuarticSolverResult result={0,{0,0,0,0}};

   QFLOAT asq = a*a ;
   QFLOAT d4 = d*doub4;
   QFLOAT p =  -b*doub2 ;
   QFLOAT q = b*b + a*c - d4 ;
   QFLOAT r = (c - a*b)*c + asq*d ;
   QFLOAT y = cubic(p,q,r) ;

   bmy = b - y ;
   y4 = y*doub4 ;
   bmysq = bmy*bmy ;
   gdis = asq - y4 ;
   hdis = bmysq - d4 ;
   if ((gdis <= doub0) || (hdis <= doub0))
	   result.num = 0;
   else{
      g1 = a*inv2 ;
      h1 = bmy*inv2 ;
      gerr = asq + y4 ;
      herr = hdis ;
      if (d > doub0) herr = bmysq + d4 ;
      if ((y < doub0) || (herr*gdis > gerr*hdis))
      {
         gdisrt = sqrt(gdis) ;
         g2 = gdisrt*inv2 ;
         if (gdisrt != doub0) h2 = (a*h1 - c)/gdisrt ;
            else h2 = doub0;
      }
      else
      {
         hdisrt = sqrt(hdis) ;
         h2 = hdisrt*inv2 ;
         if (hdisrt != doub0) g2 = (a*h1 - c)/hdisrt ;
            else g2 = doub0;
      }
     //note that in the following, the tests ensure non-zero denominators -  
      h = h1 - h2 ;
      hh = h1 + h2 ;
      hmax = hh ;
      if (hmax < doub0) hmax =  -hmax ;
      if (hmax < h) hmax = h ;
      if (hmax <  -h) hmax =  -h ;
      if ((h1 > doub0)&&(h2 > doub0)) h = d/hh ;
      if ((h1 < doub0)&&(h2 < doub0)) h = d/hh ;
      if ((h1 > doub0)&&(h2 < doub0)) hh = d/h ;
      if ((h1 < doub0)&&(h2 > doub0)) hh = d/h ;
      if (h > hmax) h = hmax ;
      if (h <  -hmax) h =  -hmax ;
      if (hh > hmax) hh = hmax ;
      if (hh <  -hmax) hh =  -hmax ;

      g = g1 - g2 ;
      gg = g1 + g2 ;
      gmax = gg ;
      if (gmax < doub0) gmax =  -gmax ;
      if (gmax < g) gmax = g ;
      if (gmax <  -g) gmax =  -g ;
      if ((g1 > doub0)&&(g2 > doub0)) g = y/gg ;
      if ((g1 < doub0)&&(g2 < doub0)) g = y/gg ;
      if ((g1 > doub0)&&(g2 < doub0)) gg = y/g ;
      if ((g1 < doub0)&&(g2 > doub0)) gg = y/g ;
      if (g > gmax) g = gmax ;
      if (g <  -gmax) g =  -gmax ;
      if (gg > gmax) gg = gmax ;
      if (gg <  -gmax) gg =  -gmax ;
 
      QuarticSolverResult result1 = quadratic(gg, hh, gg*gg - doub4*hh);
      QuarticSolverResult result2 = quadratic(g, h, g*g - doub4*h);
	  result = copyResults(result1, result2);
   }
   return result;
} 
//===neumark===


// ----------------------------------------------------------------------------
// Solve quartic equation using either
// quadratic, Ferrari's or Neumark's algorithm.
// called by
// calls  quadratic, ferrari, neumark.
// 21 Jan 1989  Don Herbison-Evans
__device__ inline QuarticSolverResult etkQuartic(const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d)
{
	QFLOAT odd, even;
	QFLOAT rts[4]={0,0,0,0};
	QuarticSolverResult r={0,{0,0,0,0}};

	if (a < doub0) odd = -a; else odd = a;
	if (c < doub0) odd -= c; else odd += c;
	if (b < doub0) even = -b; else even = b;
	if (d < doub0) even -= d; else even += d;

//First try bi-quadratic solver:
	if (odd > even*doubtol){
		int k;
		if (a < doub0) k  = 1;   else k = 0;
		if (b < doub0) k += k+1; else k +=k; 
		if (c < doub0) k += k+1; else k +=k; 
		if (d < doub0) k += k+1; else k +=k; 
		if (k==0 || k==3 || k==4 || k==6 || k==7 || k==9 || k==10 || (k>=12 && k<=15))
			r = ferrari(a,b,c,d);
		else
			r = neumark(a,b,c,d); 
	    if (r.num>0) 
			return r;
	}

//In case not success, we can still try the quadratic solver
	{
		r = quadratic(b, d, b*b - 4*d);
		QFLOAT roots[2]={r.roots[0], r.roots[1]};
		int j = 0;
		for (int k=0; k<r.num; k++){
			if (roots[k]>doub0){
				rts[j] = sqrt(roots[k]);
				rts[j+1] = -rts[j];
				++j; ++j;
			}
		}
		r.num = j;
		r.roots[0] = rts[0];
		r.roots[1] = rts[1];
		r.roots[2] = rts[2];
		r.roots[3] = rts[3];
	}
	return r;
}
