#pragma once
#ifndef NORMAL_COMPRESSER_H
#define NORMAL_COMPRESSER_H

#if defined(_MSC_VER)       //Compiling on Windows
    #if _MSC_VER <= 1700    //VS 2012 or older
        #define uint32_t unsigned __int32
    #else
        #include <cstdint>      //to provide def. for uint32_t
    #endif
#else
    #include <inttypes.h>	    //to provide def. for uint32_t
#endif


namespace MeshRender{

	inline float reinterprete_uint32_as_float(uint32_t ival)
	{
		const float * p = reinterpret_cast<const float *>(&ival);
		return *p;
	}

	//12-bit normal & 3-bit edge flags
	uint32_t compress_normal(const float normf[3], const uint32_t edgeflags = 7);

	void decompress_normal(uint32_t normi, float normf[3]);

}

#endif
