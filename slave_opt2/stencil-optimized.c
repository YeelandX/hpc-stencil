#include <athread.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

const char *version_name = "optimized-mpi-yz-overlap";

extern SLAVE_FUN(athread_comp_y_7)();
extern SLAVE_FUN(athread_halo_comp_z_7)();
extern SLAVE_FUN(athread_comp_y_7_prefetch)();

extern SLAVE_FUN(athread_comp_y_27)();
extern SLAVE_FUN(athread_halo_comp_z_27)();
extern SLAVE_FUN(athread_comp_y_27_prefetch)();

void create_dist_grid(dist_grid_info_t *grid_info, int stencil_type)
{
    /* 切分yz轴 */
    int yz_num = (int)sqrt(grid_info->p_num);
    int row_id = grid_info->p_id / yz_num;
    int col_id = grid_info->p_id % yz_num;
    grid_info->local_size_x = grid_info->global_size_x;
    grid_info->local_size_y = CROSS_BLOCK_SIZE(col_id, yz_num, grid_info->global_size_y);
    grid_info->local_size_z = CROSS_BLOCK_SIZE(row_id, yz_num, grid_info->global_size_z);
    grid_info->offset_x = 0;
    grid_info->offset_y = CROSS_BLOCK_LOW(col_id, yz_num, grid_info->global_size_y);
    grid_info->offset_z = CROSS_BLOCK_LOW(row_id, yz_num, grid_info->global_size_z);
    grid_info->halo_size_x = 1;
    grid_info->halo_size_y = 1;
    grid_info->halo_size_z = 1;
    athread_init();
}

void destroy_dist_grid(dist_grid_info_t *grid_info) {}

ptr_t stencil_7(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt)
{
    ptr_t buffer[2] = {grid, aux};
    int x_start = grid_info->halo_size_x,
        x_end = grid_info->local_size_x + grid_info->halo_size_x;
    int y_start = grid_info->halo_size_y,
        y_end = grid_info->local_size_y + grid_info->halo_size_y;
    int z_start = grid_info->halo_size_z,
        z_end = grid_info->local_size_z + grid_info->halo_size_z;
    int ldx = grid_info->local_size_x + 2 * grid_info->halo_size_x;
    int ldy = grid_info->local_size_y + 2 * grid_info->halo_size_y;
    int inner_z_start = z_start + 1, inner_z_end = z_end - 1,
        inner_z_size = inner_z_end - inner_z_start;
    int inner_y_start = y_start + 1, inner_y_end = y_end - 1,
        inner_y_size = inner_y_end - inner_y_start;
    int t, y, x;

    for (t = 0; t < nt; ++t)
    {
        cptr_t a0 = buffer[t % 2];
        ptr_t a1 = buffer[(t + 1) % 2];

        // inner
        comp_param_t inner_param;
        init_comp_param_t(&inner_param, a1, a0, x_start, grid_info->local_size_x,
                          inner_y_start, inner_y_size, inner_z_start,
                          inner_z_size, ldx, ldy);
        athread_spawn(athread_comp_y_7_prefetch, &inner_param);

        halo_comm_yz_sync(grid_info, a0);

        comp_param_t halo_up_param;
        init_comp_param_t(&halo_up_param, a1, a0, x_start, grid_info->local_size_x,
                          y_start, grid_info->local_size_y, z_start, 1, ldx, ldy);
        comp_param_t halo_down_param;
        init_comp_param_t(&halo_down_param, a1, a0, x_start,
                          grid_info->local_size_x, y_start, grid_info->local_size_y,
                          inner_z_end, 1, ldx, ldy);

        comp_param_t halo_front_param;
        init_comp_param_t(&halo_front_param, a1, a0, x_start, grid_info->local_size_x,
                          y_start, 1, inner_z_start, inner_z_size, ldx, ldy);
        comp_param_t halo_back_param;
        init_comp_param_t(&halo_back_param, a1, a0, x_start, grid_info->local_size_x,
                          inner_y_end, 1, inner_z_start, inner_z_size, ldx, ldy);

        athread_join();

        athread_spawn(athread_comp_y_7, &halo_up_param);
        athread_join();
        athread_spawn(athread_comp_y_7, &halo_down_param);
        athread_join();
        athread_spawn(athread_halo_comp_z_7, &halo_front_param);
        athread_join();
        athread_spawn(athread_halo_comp_z_7, &halo_back_param);
        athread_join();
    }

    return buffer[nt % 2];
}

ptr_t stencil_27(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info,
                 int nt)
{
    ptr_t buffer[2] = {grid, aux};
    int x_start = grid_info->halo_size_x,
        x_end = grid_info->local_size_x + grid_info->halo_size_x;
    int y_start = grid_info->halo_size_y,
        y_end = grid_info->local_size_y + grid_info->halo_size_y;
    int z_start = grid_info->halo_size_z,
        z_end = grid_info->local_size_z + grid_info->halo_size_z;
    int ldx = grid_info->local_size_x + 2 * grid_info->halo_size_x;
    int ldy = grid_info->local_size_y + 2 * grid_info->halo_size_y;
    int inner_z_start = z_start + 1, inner_z_end = z_end - 1,
        inner_z_size = inner_z_end - inner_z_start;
    int inner_y_start = y_start + 1, inner_y_end = y_end - 1,
        inner_y_size = inner_y_end - inner_y_start;
    int t, y, x;
    for (t = 0; t < nt; ++t)
    {
        cptr_t a0 = buffer[t % 2];
        ptr_t a1 = buffer[(t + 1) % 2];

        // inner
        comp_param_t inner_param;
        init_comp_param_t(&inner_param, a1, a0, x_start, grid_info->local_size_x,
                          inner_y_start, inner_y_size, inner_z_start,
                          inner_z_size, ldx, ldy);
        athread_spawn(athread_comp_y_27_prefetch, &inner_param);

        halo_comm_yz_sync(grid_info, a0);

        comp_param_t halo_up_param;
        init_comp_param_t(&halo_up_param, a1, a0, x_start, grid_info->local_size_x,
                          y_start, grid_info->local_size_y, z_start, 1, ldx, ldy);
        comp_param_t halo_down_param;
        init_comp_param_t(&halo_down_param, a1, a0, x_start,
                          grid_info->local_size_x, y_start, grid_info->local_size_y,
                          inner_z_end, 1, ldx, ldy);
        comp_param_t halo_front_param;
        init_comp_param_t(&halo_front_param, a1, a0, x_start, grid_info->local_size_x,
                          y_start, 1, inner_z_start, inner_z_size, ldx, ldy);
        comp_param_t halo_back_param;
        init_comp_param_t(&halo_back_param, a1, a0, x_start, grid_info->local_size_x,
                          inner_y_end, 1, inner_z_start, inner_z_size, ldx, ldy);

        athread_join();

        athread_spawn(athread_comp_y_27, &halo_up_param);
        athread_join();
        athread_spawn(athread_comp_y_27, &halo_down_param);
        athread_join();
        athread_spawn(athread_halo_comp_z_27, &halo_front_param);
        athread_join();
        athread_spawn(athread_halo_comp_z_27, &halo_back_param);
        athread_join();
    }
    return buffer[nt % 2];
}