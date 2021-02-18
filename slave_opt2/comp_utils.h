#ifndef COMP_UTILS_H
#define COMP_UTILS_H
#include <simd.h>

#define my_simd_loadu(v, p) ({ v = simd_set_doublev4(((double *)p)[0], ((double *)p)[1], ((double *)p)[2], ((double *)p)[3]); })

void vector_vmuld_32(double *o, double *v, int len, double alpha);
void vector_vmad_32(double *o, double *v, int len,double alpha);
void vector_vmad_u_32(double *o, double *v, double *temp, int len, double alpha);

#endif // !COMP_UTILS_H