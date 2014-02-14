const uint NORMAL_BITS = 12u;
const uint NORMAL_MAXI = (1u << NORMAL_BITS) - 1u;
const uint NORMAL_MAXI_2 = NORMAL_MAXI >> 1;

#if !defined(__STDC__) && !defined(__cplusplus)

vec3 decompress_normal(uint normi)
{
    uint u = uint(normi & NORMAL_MAXI);
	uint v = uint((normi >> NORMAL_BITS) & NORMAL_MAXI_2);
    if (u + v >= NORMAL_MAXI){
        u = NORMAL_MAXI - u;
        v = NORMAL_MAXI - v;
    }
    uint w = uint(NORMAL_MAXI - 1u - u - v);
	vec3 n = vec3(uvec3(u, v, w));
	uvec3 signs = uvec3(normi>>29, normi>>30, normi>>31) & 1u;
	return normalize(mix(n, -n, vec3(signs)));
}

#endif