
#ifndef __ZVECTOR2I__
#define __ZVECTOR2I__

#include <math.h>


class Vector2I 
{

public:

    int x, y;

    // constructors
    Vector2I() { }

    Vector2I(const int X, const int Y) 
	{ 
		x = X; y = Y;
	}

    Vector2I(const int X) { x = y = X; }

    Vector2I(const Vector2I& v) 
	{ 
		x = v.x; y = v.y; 
	}

    // Functions to get at the vector components
    int& operator[] (int index) 
	{
		int *p = &x;
		return p[index];
    }
	//
    const int& operator[] (int index) const 
	{
		const int *p = &x;
		return p[index];
    }

	void BubbleSort(void)
	{
		if (x>y){
			int t = x;
			x = y; y = t;
		}
	}
};


///================================================

class Vector4I 
{

public:

    int x, y, z, w;

    // constructors
    Vector4I() { }

    Vector4I(const int X, const int Y, const int Z, const int W) 
	{ 
		x = X; y = Y; z = Z; w=W;
	}

    Vector4I(const int X) { x = y = z = w = X; }

    Vector4I(const Vector4I& v) 
	{ 
		x = v.x; y = v.y; z = v.z; 
		w = v.w;
	}

    // Functions to get at the vector components
    int& operator[] (int inx) 
	{
		int *p = &x;
		return p[inx];
    }
	//
    const int& operator[] (int inx) const 
	{
		const int *p = &x;
		return p[inx];
    }

	void BubbleSort(void)
	{
		_bubbleSortInt(&x, 4);
	}
};




///================================================

class Vector8I 
{

public:

    int x, y, z, w;
	int x1, y1, z1, w1;

    // constructors
    Vector8I() 
	{ 
		x=y=z=w=x1=y1=z1=w1=0;
	}

    Vector8I(const int X, const int Y, const int Z, const int W, 
			 const int X1, const int Y1, const int Z1, const int W1
		) 
	{ 
		x = X; y = Y; z = Z; w=W;
		x1 = X1; y1 = Y1; z1 = Z1; w1=W1;
	}

    Vector8I(const int X) 
	{ 
		x = y = z = w = X; 
		x1 = y1 = z1 = w1 = X; 
	}

    Vector8I(const Vector8I& v) 
	{ 
		x = v.x; y = v.y; z = v.z; w = v.w;
		x1 = v.x1; y1 = v.y1; z1 = v.z1; w1 = v.w1;
	}

	void BubbleSort(void)
	{
		_bubbleSortInt(&x, 8);
	}


    // Functions to get at the vector components
    int& operator[] (int inx) 
	{
		int *p = &x;
		return p[inx];
    }
	//
    const int& operator[] (int inx) const 
	{
		const int *p = &x;
		return p[inx];
    }
};

#endif
