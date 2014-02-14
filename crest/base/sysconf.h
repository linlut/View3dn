/*
File: sysconf.h

  Define system dependent configration.
  Difference in Win32 and UNIX is only considered.

  Nan Zhang   Nov.30, 2001
*/


#ifndef _SYSTEM_CONFIG_H_	
#define _SYSTEM_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <assert.h>


#ifdef WIN32
	#include <malloc.h>
	#include <iostream>
	#include <fstream>
	#include <iomanip>
	using namespace std;
#else
	#include <iostream>
	#include <fstream>
	#include <iomanip>
#endif

//We define this debug MACRO to generate debug code
#ifdef WIN32
    //fopen using binary mode
    #define _RB_ "rb"   
    //fopen using ASCII mode
    #define _RA_ "r"   
    //fwrite using binary mode
    #define _WB_ "wb"
    //fwrite using ASCII mode
    #define _WA_ "w"

#else
	//fopen using binary mode
	#define _RB_ "r"   
	//fopen using ASCII mode
	#define _RA_ "r"   
	//fwrite using binary mode
	#define _WB_ "w"
	//fwrite using ASCII mode
	#define _WA_ "w"
#endif



#ifndef _MAX_
	#define _MAX_(a,b) (((a) > (b)) ?  (a) : (b))
	#define _MIN_(a,b) (((a) < (b)) ?  (a) : (b))
	#define _MAX3_(a,b,c) (((a) > (b)) ?  (_MAX_((a),(c))) : (_MAX_((b),(c))))
	#define _MIN3_(a,b,c) (((a) < (b)) ?  (_MIN_((a),(c))) : (_MIN_((b),(c))))
#endif


#ifndef PI
#define PI 3.14159265359
#endif 

#ifndef M_PI
#define M_PI 3.14159265359
#endif 

#ifndef MAXFLOAT 
#define MAXFLOAT ((float)1e38)
#endif

//typedef int INTARRAY3[3];

#ifdef _DEBUG  
	inline void ASSERT0( bool X)
	{
		if (!X){
			printf("ASSERTION Failed!\n");
 			assert(X);  
		}
	}
#else
	#define ASSERT0(X)  
#endif


inline void VALID_POINTER(void *p)
{
#ifdef _DEBUG
	ASSERT0(p!=NULL);
	unsigned long t = (long)p;
	ASSERT0(t!=-1l);
	ASSERT0((t&0xff000000)!=0xcc000000);
	ASSERT0((t&0xf0000000)!=0xf0000000);
#endif
}


#ifdef _DEBUG
	#define PERROR(x) {printf(x); fflush(stdout);}
#else
	#define PERROR(x)
#endif


inline void NOT_IMPLEMENTED(void)
{
	printf("Not implemented\n");
	assert(0);
}


inline void PRINT_BINARY_DWORD(const unsigned int word)
{
	unsigned int w = word;

	for (int i=31; i>=0; i--){
		int b = (w>>i)&0x1;
		printf("%d", b);
		if (i%4==0 && i!=0)
			printf(":");
	}
}


inline void PRINT_BINARY_WORD(const unsigned int word)
{
	unsigned int w = word;

	for (int i=15; i>=0; i--){
		int b = (w>>i)&0x1;
		printf("%d", b);
		if (i%4==0 && i!=0)
			printf(":");
	}
}



#ifdef _DEBUG  

	inline void TEST_MEMORY(void)
	{
		float *p = new float [32768];
		if (p==NULL)
			assert(0);
		else
			delete [] p;
	}

#else
	#define TEST_MEMORY(X)  
#endif



#ifndef SafeDelete
	#define SafeDelete(p) if (p) { delete p; p = NULL; }
	#define SafeDeleteArray(p) if (p) { delete [] p; p = NULL;}
#endif




#endif	// _CONFIG_H_
