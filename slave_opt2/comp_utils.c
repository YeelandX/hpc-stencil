#include "comp_utils.h"
#include <simd.h>

void vector_vmuld_32(double *o, double *v, int len, double alpha)
{
    register double *vv, *oo;
    register doublev4 o1, o2, o3, o4, o5, o6, o7, o8,
        v1, v2, v3, v4, v5, v6, v7, v8;
    register doublev4 ALPHA = simd_set_doublev4(alpha, alpha, alpha, alpha);
    register int ox;
    for (ox = 0; ox < len; ox += 32)
    {
        vv = v + ox;
        oo = o + ox;
        simd_load(v1, vv);
        simd_load(v2, vv + 4);
        simd_load(v3, vv + 8);
        simd_load(v4, vv + 12);
        simd_load(v5, vv + 16);
        simd_load(v6, vv + 20);
        simd_load(v7, vv + 24);
        simd_load(v8, vv + 28);

        o1 = simd_vmuld(ALPHA, v1);
        o2 = simd_vmuld(ALPHA, v2);
        o3 = simd_vmuld(ALPHA, v3);
        o4 = simd_vmuld(ALPHA, v4);
        o5 = simd_vmuld(ALPHA, v5);
        o6 = simd_vmuld(ALPHA, v6);
        o7 = simd_vmuld(ALPHA, v7);
        o8 = simd_vmuld(ALPHA, v8);

        simd_store(o1, oo);
        simd_store(o2, oo + 4);
        simd_store(o3, oo + 8);
        simd_store(o4, oo + 12);
        simd_store(o5, oo + 16);
        simd_store(o6, oo + 20);
        simd_store(o7, oo + 24);
        simd_store(o8, oo + 28);
    }
}

void vector_vmad_32(double *o, double *v, int len,double alpha)
{
    register double *vv, *oo;
    register doublev4 o1, o2, o3, o4, o5, o6, o7, o8,
        v1, v2, v3, v4, v5, v6, v7, v8;
    register doublev4 ALPHA = simd_set_doublev4(alpha, alpha, alpha, alpha);
    register int ox;
    for (ox = 0; ox < len; ox += 32)
    {
        vv = v + ox;
        oo = o + ox;
        simd_load(o1, oo);
        simd_load(o2, oo + 4);
        simd_load(o3, oo + 8);
        simd_load(o4, oo + 12);
        simd_load(o5, oo + 16);
        simd_load(o6, oo + 20);
        simd_load(o7, oo + 24);
        simd_load(o8, oo + 28);

        simd_load(v1, vv);
        simd_load(v2, vv + 4);
        simd_load(v3, vv + 8);
        simd_load(v4, vv + 12);
        simd_load(v5, vv + 16);
        simd_load(v6, vv + 20);
        simd_load(v7, vv + 24);
        simd_load(v8, vv + 28);

        o1 = simd_vmad(ALPHA, v1, o1);
        o2 = simd_vmad(ALPHA, v2, o2);
        o3 = simd_vmad(ALPHA, v3, o3);
        o4 = simd_vmad(ALPHA, v4, o4);
        o5 = simd_vmad(ALPHA, v5, o5);
        o6 = simd_vmad(ALPHA, v6, o6);
        o7 = simd_vmad(ALPHA, v7, o7);
        o8 = simd_vmad(ALPHA, v8, o8);

        simd_store(o1, oo);
        simd_store(o2, oo + 4);
        simd_store(o3, oo + 8);
        simd_store(o4, oo + 12);
        simd_store(o5, oo + 16);
        simd_store(o6, oo + 20);
        simd_store(o7, oo + 24);
        simd_store(o8, oo + 28);
    }
}

void vector_vmad_u_32(double *o, double *v, double *temp, int len, double alpha)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        temp[i] = v[i];
    }
    vector_vmad_32(o, temp, len, alpha);
}