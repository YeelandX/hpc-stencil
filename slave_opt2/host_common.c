#include "common.h"

inline void init_comp_param_t(comp_param_t *param, ptr_t a1, cptr_t a0,
                              int x_start, int x_size, int y_start, int y_size,
                              int z_start, int z_size, int ldx, int ldy)
{
    param->a0 = a0;
    param->a1 = a1;
    param->x_start = x_start;
    param->x_size = x_size;
    param->y_start = y_start;
    param->y_size = y_size;
    param->z_start = z_start;
    param->z_size = z_size;
    param->ldx = ldx;
    param->ldy = ldy;
}
