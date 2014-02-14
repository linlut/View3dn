//FILE: vector2i.h

#ifndef __ZVECTOR2I__
#define __ZVECTOR2I__

#include <math.h>
#include <vector2f.h>

//=======vector2i definition=======================
typedef _Vector2T<int> Vector2i;
typedef Vector2i Vector2I;

///=================================================
inline void _bubbleSortInt(int a[], const int n)
{
	for (int i=0; i<n-1; i++) {
		for (int j=0; j<n-1-i; j++){
			if (a[j+1] < a[j]) {  
				int tmp = a[j];   
				a[j] = a[j+1];
				a[j+1] = tmp;
			}
		}
	}
}


///================================================
class Vector8I 
{

public:

    int x, y, z, w;
	int x1, y1, z1, w1;

    // constructors
    Vector8I() 
	{ 
		//x=y=z=w=x1=y1=z1=w1=0;
	}

    inline Vector8I(const int X, const int Y, const int Z, const int W, 
			 const int X1, const int Y1, const int Z1, const int W1) 
	{ 
		x = X; y = Y; z = Z; w=W;
		x1 = X1; y1 = Y1; z1 = Z1; w1=W1;
	}

    inline Vector8I(const int X) 
	{ 
		x = y = z = w = X; 
		x1 = y1 = z1 = w1 = X; 
	}

    inline Vector8I(const Vector8I& v) 
	{ 
		x = v.x; y = v.y; z = v.z; w = v.w;
		x1 = v.x1; y1 = v.y1; z1 = v.z1; w1 = v.w1;
	}

	inline void BubbleSort(void)
	{
		_bubbleSortInt(&x, 8);
	}


    // Functions to get at the vector components
    inline int& operator[] (const int inx) 
	{
		int *p = &x;
		return p[inx];
    }
	//
    inline const int& operator[] (const int inx) const 
	{
		const int *p = &x;
		return p[inx];
    }

    // Binary operators======================================
    inline friend Vector8I operator + (const Vector8I& A, const Vector8I& B)
	{
		return Vector8I(A.x+B.x, A.y+B.y, A.z+B.z, A.w+B.w, 
						A.x1+B.x1, A.y1+B.y1, A.z1+B.z1, A.w1+B.w1);
	}

    inline friend Vector8I operator - (const Vector8I& A, const Vector8I& B)
	{
		return Vector8I(A.x-B.x, A.y-B.y, A.z-B.z, A.w-B.w, 
						A.x1-B.x1, A.y1-B.y1, A.z1-B.z1, A.w1-B.w1);
	}

};


#endif
