#if !defined(__STDC__) && !defined(__cplusplus)

//shading a light 
vec3 shadeVertex(vec4 vertex, vec3 normal, vec3 facecolor)
{
    vec3 fragcolor = facecolor * 0.75;
    if (getShadingFlag()){
        vec3 viewdir = vec3(1.0, 1.0, 0.10);
        viewdir = normalize(viewdir);

        if (getFlipNormalFlag()) normal = -normal;

        float diffuse = abs(dot(viewdir, normal)) + 0.20;
        fragcolor = facecolor * diffuse;
    }
    return fragcolor;
}

#endif
