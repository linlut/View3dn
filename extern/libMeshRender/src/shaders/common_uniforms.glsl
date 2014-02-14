uniform vec4 uniformsf[6];
uniform ivec4 uniformsint[1];

#define FACE_FRONT_COLOR_ROW    0
#define FACE_BACK_COLOR_ROW     1
#define LINE_COLOR_ROW          2
#define SCREEN_SIZE_ROW         3
#define ATTRIBUTE_MINMAX_ROW    4


#if !defined(__STDC__) && !defined(__cplusplus)

vec3 getFaceFrontColor(){
    int i = FACE_FRONT_COLOR_ROW;
    return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);
}

vec3 getFaceBackColor(){
    int i = FACE_BACK_COLOR_ROW;
    return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);
}

vec3 getLineColor(){
    int i = LINE_COLOR_ROW;
    return vec3(uniformsf[i].x, uniformsf[i].y, uniformsf[i].z);
}

float getLineWidth(){
    const int i = LINE_COLOR_ROW;
    return uniformsf[i].w;
}

vec2 getScreenSize(){
    const int i = SCREEN_SIZE_ROW;
    vec2 r = vec2(uniformsf[i].x, uniformsf[i].y);
    return r;
}

vec2 getScreenSizeInv(){
    const int i = SCREEN_SIZE_ROW;
    vec2 r = vec2(uniformsf[i].z, uniformsf[i].w);
    return r;
}

vec2 getAttributeMinMax(){
    const int i = ATTRIBUTE_MINMAX_ROW;
    return vec2(uniformsf[i].x, uniformsf[i].y);
}

bool getHiddenLineFlag(){
    return (uniformsint[0].x & 1) == 1;
}

bool getShadingFlag(){
    return (uniformsint[0].x & 2) == 2;
}

bool getTextureFlag(){
    return (uniformsint[0].x & 4) == 4;
}

bool getFlipNormalFlag(){
    return (uniformsint[0].x & 8) == 8;
}


//function for hiddenline rendering
vec3 blendLineColorByDistance(vec3 ptdist, vec3 facecolor){
    vec3 fragcolor = facecolor;
    bool hiddenLineFlag = getHiddenLineFlag();
    if (hiddenLineFlag){    
        vec3 linecolor = getLineColor();
        float linewidth = getLineWidth();
        float d = min(ptdist[0], min(ptdist[1], ptdist[2]));
        d -= min(linewidth * 0.5, linewidth-1.0);
        d = max(0.0, d);
        float I = exp(-d * d);
        fragcolor = mix(fragcolor, linecolor, I);
    }
    return fragcolor;
}

#endif