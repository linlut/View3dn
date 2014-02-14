//////////////////////////////////////////////////////////////////////////////
/// MedVR Simulation
/// S.Y.H.
/// Virtual Environment Lab
/// Biomedical Engineering
/// Electrical Engineering
/// UTSW, UTA
/////////////////////////////////////////////////////////////////////////////
#include "mathdefs.h"

void MultVectorByMatrix(float m[], vertex *v,vertex *result)
{
	result->x = (m[0] * v->x) +
			   (m[4] * v->y) +	
			   (m[8] * v->z) +
			   m[12];
	result->y = (m[1] * v->x) +
			   (m[5] * v->y) +	
			   (m[9] * v->z) +
			   m[13];
	result->z = (m[2] * v->x) +
			   (m[6] * v->y) +	
			   (m[10] * v->z) +
			   m[14];
}
/*// MultVectorByMatrix //////////////////////////////////////////////////////
void MultVectorByMatrix(tMatrix *mat, vertex *v,vertex *result)
{
	result->x = (mat->m[0] * v->x) +
			   (mat->m[4] * v->y) +	
			   (mat->m[8] * v->z) +
			   mat->m[12];
	result->y = (mat->m[1] * v->x) +
			   (mat->m[5] * v->y) +	
			   (mat->m[9] * v->z) +
			   mat->m[13];
	result->z = (mat->m[2] * v->x) +
			   (mat->m[6] * v->y) +	
			   (mat->m[10] * v->z) +
			   mat->m[14];
}
*/

/* returns squared length of input vector */    
double VectorSquaredLength(vertex *v) 
{
	return((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

/* returns length of input vector */
double VectorLength(vertex *v) 
{
	return(sqrt(VectorSquaredLength(v)));
}

/* destructively normalizes the input vector */
void NormalizeVector(vertex *v) 
{
	float len = (float)VectorLength(v);
    if (len != 0.0) 
	{ 
		v->x /= len;  
		v->y /= len; 
		v->z /= len; 
	}
}

double DotProduct(vertex *v1, vertex *v2)
{
	return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

/* return the cross product result = v1 cross v2 */
void CrossProduct(vertex *v1, vertex *v2, vertex *result)
{
	result->x = (v1->y * v2->z) - (v1->z * v2->y);
	result->y = (v1->z * v2->x) - (v1->x * v2->z);
	result->z = (v1->x * v2->y) - (v1->y * v2->x);
}

double VectorSquaredDistance(vertex *v1, vertex *v2) 
{
	return(	((v1->x - v2->x) * (v1->x - v2->x)) + 
			((v1->y - v2->y) * (v1->y - v2->y)) + 	
			((v1->z - v2->z) * (v1->z - v2->z)) ); 	
}

void ScaleVector(vertex *v, float scale, vertex *result) 
{
	result->x = v->x * scale;
	result->y = v->y * scale;
	result->z = v->z * scale;
}

void VectorSum(vertex *v1, vertex *v2, vertex *result) 
{
	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
	result->z = v1->z + v2->z;
}

void VectorDifference(vertex *v1, vertex *v2, vertex *result) 
{
	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
	result->z = v1->z - v2->z;
}

void Multiply44(float* a, float* b)
{
	int r,c;
	float tmp[16];

	for(r=0;r<4;r++)
		for(c=0;c<4;c++) tmp[r*4+c]=a[r*4+0]*b[0*4+c]+a[r*4+1]*b[1*4+c]+a[r*4+2]*b[2*4+c]+a[r*4+3]*b[3*4+c];
	for(r=0;r<4;r++)
		for(c=0;c<4;c++) b[r*4+c]=tmp[r*4+c];
}

void SetIdentityMatrix(float* m)
{
	int i,j;
	for (i=0;i<4;i++)
		for (j=0;j<4;j++) m[i*4+j]=(i==j);
}

void SetTranslateMatrix(float* m, vertex t)
{
	SetIdentityMatrix(m);
	m[3]=t.x;
	m[7]=t.y;
	m[11]=t.z;
}

void SetTranslateMatrix2(float* m, float x, float y, float z)
{
	SetIdentityMatrix(m);
	m[3]=x;
	m[7]=y;
	m[11]=z;
}

void SetScaleMatrix(float* m, float x, float y, float z, vertex center)
{
	SetIdentityMatrix(m);
	m[0]=x;
	m[3]=(1-x) * center.x;
	m[5]=y;
	m[7]=(1-y) * center.y;
	m[10]=z;
	m[11]=(1-z) * center.z; 
}

void SetScaleMatrix(float* m, float x, float y, float z)
{
	SetIdentityMatrix(m);
	m[0]=x;
	m[5]=y;
	m[10]=z;
}

void SetRotateMatrix(float* m, vertex p1, vertex p2, float radians)
{
	float R[16], T[16];
	SetIdentityMatrix(R);
	float length=sqrt(	(p2.x-p1.x)*(p2.x-p1.x)+
						(p2.y-p1.y)*(p2.y-p1.y)+
						(p2.z-p1.z)*(p2.z-p1.z) );
	float cosa=cos(radians/2.0);
	float sina=sin(radians/2.0);
	float a=sina*(p2.x-p1.x)/length;
	float b=sina*(p2.y-p1.y)/length;
	float c=sina*(p2.z-p1.z)/length;
	SetTranslateMatrix2(m, -p1.x, -p1.y, -p1.z);
	SetTranslateMatrix2(T, p1.x, p1.y, p1.z);
	SetIdentityMatrix(R);
	R[0]=1.0-2*b*b-2*c*c;
	R[1]=2*a*b-2*cosa*c;
	R[2]=2*a*c+2*cosa*b;
	R[4]=2*a*b+2*cosa*c;
	R[5]=1.0-2*a*a-2*c*c;
	R[6]=2*b*c-2*cosa*a;
	R[8]=2*a*c-2*cosa*b;
	R[9]=2*b*c+2*cosa*a;
	R[10]=1.0-2*a*a-2*b*b;
	Multiply44(R, m);
	Multiply44(T, m);
}

void Transform(float* m, vertex* v0, vertex* v, long nLength)
{
	long i;
	for(i=0;i<nLength;i++)
	{
		v[i].x=v0[i].x*m[0*4+0]+v0[i].y*m[0*4+1]+v0[i].z*m[0*4+2]+m[0*4+3];
		v[i].y=v0[i].x*m[1*4+0]+v0[i].y*m[1*4+1]+v0[i].z*m[1*4+2]+m[1*4+3];
		v[i].z=v0[i].x*m[2*4+0]+v0[i].y*m[2*4+1]+v0[i].z*m[2*4+2]+m[2*4+3];
	}
}

void Transforms(float* m, vertex* v, long nLength)
{
	long i;
	vertex v0;
	for(i=0;i<nLength;i++)
	{
		v0=v[i];
		v[i].x=v0.x*m[0*4+0]+v0.y*m[0*4+1]+v0.z*m[0*4+2]+m[0*4+3];
		v[i].y=v0.x*m[1*4+0]+v0.y*m[1*4+1]+v0.z*m[1*4+2]+m[1*4+3];
		v[i].z=v0.x*m[2*4+0]+v0.y*m[2*4+1]+v0.z*m[2*4+2]+m[2*4+3];
	}
}


void SetOriginRotateMatrix(float* m, vertex radians)
{
	float fRx[16], fRy[16], fRz[16];
	SetIdentityMatrix(fRx);
	SetIdentityMatrix(fRy);
	SetIdentityMatrix(fRz);
	SetIdentityMatrix(m);

	fRx[0]=1; 
	fRx[5]=cos(radians.x);
	fRx[6]=-1*sin(radians.x);
	fRx[9]=-1*fRx[6];
	fRx[10]=fRx[5];
	fRx[15]=1;
	//
	fRy[0]=cos(radians.y); 
	fRy[2]=sin(radians.y);
	fRy[5]=1;
	fRy[8]=-1*fRy[2];
	fRy[10]=fRy[0];
	fRy[15]=1;
	//
	fRz[0]=cos(radians.z); 
	fRz[1]=-1*sin(radians.z);
	fRz[4]=-1*fRz[1];
	fRz[5]=fRz[0];
	fRz[10]=1;
	fRz[15]=1;
	// Must follow z/x/y order due to phantom handle design;
	Multiply44(fRz, m);
	Multiply44(fRx, m);
	Multiply44(fRy, m);
}

vertex* CalcNormal(vertex* v1, vertex* v2, vertex* v3, vertex* norm, int iNormalize)
{
	float a[3],b[3];
	
	a[0] = v1->x - v2->x;
	a[1] = v1->y - v2->y;
	a[2] = v1->z - v2->z;

	b[0] = v2->x - v3->x;
	b[1] = v2->y - v3->y;
	b[2] = v2->z - v3->z;

	norm->x=a[1]*b[2]-a[2]*b[1];
	norm->y=a[2]*b[0]-a[0]*b[2];
	norm->z=a[0]*b[1]-a[1]*b[0];

	if(iNormalize) return Norm(norm);
	else return norm;
}

vertex* Norm(vertex* vector)
{
	float length;
	length=(float)sqrt(vector->x*vector->x+vector->y*vector->y+vector->z*vector->z);
	if(length)
	{
		vector->x/=length;
		vector->y/=length;
		vector->z/=length;
	}
	return vector;
}