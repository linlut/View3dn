#include <iostream>
#include <math.h>
#include "vector3f.h"
#include "NormalCompresser.h"

typedef uint32_t uint;
#include "shaders/normal_decompression.glsl"


namespace MeshRender{

	inline float signed_value(float r, uint sign)
	{
		if (sign != 0) r = -r;
		return r;
	}

	inline float signed_one(uint sign)
	{
		return signed_value(1.0f, sign);
	}

    //See http://oroboro.com/compressed-unit-vectors/
    //for the detailed explanation of the algorithm
	uint32_t compress_normal(const float normf[3], const uint edgeflags)
	{
		const uint32_t signx = (uint32_t)(normf[0] < 0);
		const uint32_t signy = (uint32_t)(normf[1] < 0);
		const uint32_t signz = (uint32_t)(normf[2] < 0);
		const uint32_t signs = signx + (signy << 1) + (signz << 2);
        
		const float a = fabs(normf[0]);
		const float b = fabs(normf[1]);
		const float c = fabs(normf[2]);
    	const float NORMAL_MAXF = float(NORMAL_MAXI) - 1.0f;
		const float s = NORMAL_MAXF / (a + b + c);

		uint32_t u = (uint32_t)(a * s);
		uint32_t v = (uint32_t)(b * s);
        if (v > NORMAL_MAXI_2){
            u = NORMAL_MAXI - u;
            v = NORMAL_MAXI - v;
        }

		return (signs << 29) |
			(2u << (NORMAL_BITS * 2 + 3)) |
			(edgeflags << (NORMAL_BITS * 2 - 1)) |
			(v << NORMAL_BITS) |
			u;
	}

	void decompress_normal(uint32_t normi, float normf[3])
	{
		int u = normi & NORMAL_MAXI;
		int v = (normi >> NORMAL_BITS) & NORMAL_MAXI_2;
        if (u + v >= NORMAL_MAXI){
            u = NORMAL_MAXI - u;
            v = NORMAL_MAXI - v;
        }
		const int w = (NORMAL_MAXI - 1 - u - v);
		Vector3f& n = *((Vector3f*)normf);
		n = Vector3f(float(u), float(v), float(w));
		n.normalize();

        //set signs of the normal components
		uint32_t signs = normi >> 29;
		uint32_t signx = signs & 1u;
		uint32_t signy = (signs >> 1) & 1u;
		uint32_t signz = (signs >> 2) & 1u;
		n.x = signed_value(n.x, signx);
		n.y = signed_value(n.y, signy);
		n.z = signed_value(n.z, signz);
	}

#if defined(_DEBUG) || defined(DEBUG)
	void print_int_binary(unsigned int x)
	{
		const int n = sizeof(x) * 8; 
		char buffer[n + 20];
		int c = 0;
		for (int i=0; i<n; ++i){
			unsigned int v = (x >> (n - i - 1)) & 1; 
			buffer[c++] = '0' + v;
			//if ((i & 3) == 3 && i != (n-1)){
			if (i==2 || i==5 || i==18){
				buffer[c++] = '-';
			}
		}

		buffer[c] = 0;
		std::cout << buffer;
	}

	int unit_test()
	{
		const int N=10;
		for (int j=0; j<N; j++){
			const double beta = M_PI * ((double)j / N - 0.5);
			const double sinbeta = sin(beta);
			const double cosbeta = cos(beta);
			for (int i=0; i<N*2; i++){
				const double alpha = 2.0 * M_PI * (double)i / (N *2);
				const double cosalpha = cos(alpha);
				const double sinalpha = sin(alpha);
				const Vector3f innorm(float(cosalpha*cosbeta), float(sinalpha*cosbeta), float(sinbeta));
				const unsigned int ni = compress_normal(&innorm.x);
				float * nf = (float *)(&ni);
				Vector3f n(0);
				decompress_normal(ni, &n.x);
				const float r = DotProd(n, innorm);
				if (r < .9999){
					print_int_binary(ni);
					std::cout << ": " << i 
						<< ": " << *nf
						<< ": " << r
						<< ", " << innorm.x
						<< ", " << innorm.y
						<< ", " << innorm.z
						<< std::endl;
				}
			}
		}
		return 1;
	}

    static int test_value = MeshRender::unit_test();

#endif

}//namespace

