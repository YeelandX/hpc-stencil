#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <mpi.h>

const char *version_name = "Optimized version";
extern void funcee();
extern mm_param mmp;
/* your implementation */
void create_dist_grid(dist_grid_info_t *grid_info, int stencil_type)
{
    int p_num = grid_info->p_num;
    int p_id = grid_info->p_id;

    //printf("p_id=%d,p_num=%d\n", p_id, p_num);
    grid_info->local_size_x = grid_info->global_size_x;
    grid_info->local_size_y = grid_info->global_size_y;
    grid_info->local_size_z = grid_info->global_size_z / p_num;

    // int num_x=2;
    // int num_y=2;
    // int num_z=4;
    // grid_info->offset_x = ((p_id % (4)) % 2) * (grid_info->local_size_x);
    // grid_info->offset_y = ((p_id % (4)) / 2) * (grid_info->local_size_y);
    // grid_info->offset_z = (p_id / (4)) * (grid_info->local_size_z);
    grid_info->offset_x = 0;
    grid_info->offset_y = 0;
    grid_info->offset_z = p_id * (grid_info->local_size_z);

    grid_info->halo_size_x = 1;
    grid_info->halo_size_y = 1;
    grid_info->halo_size_z = 1;
    //puts("not implemented");
    //exit(1);
}

/* your implementation */
void destroy_dist_grid(dist_grid_info_t *grid_info)
{
}

/* your implementation */
ptr_t stencil_7(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt)
{
    ptr_t buffer[2] = {grid, aux};
    int x_start = grid_info->halo_size_x, x_end = grid_info->local_size_x + grid_info->halo_size_x;
    int y_start = grid_info->halo_size_y, y_end = grid_info->local_size_y + grid_info->halo_size_y;
    int z_start = grid_info->halo_size_z, z_end = grid_info->local_size_z + grid_info->halo_size_z;

    int ldx = grid_info->local_size_x + 2 * grid_info->halo_size_x;
    int ldy = grid_info->local_size_y + 2 * grid_info->halo_size_y;
    int ldz = grid_info->local_size_z + 2 * grid_info->halo_size_z;
    int p_id = grid_info->p_id;
    int ls = ldx * ldy;
    MPI_Status status;
    MPI_Request request;
    mmp.ldx=ldx;
    mmp.ldy=ldy;
    mmp.ldz=ldz;
    mmp.x_start=x_start;
    mmp.y_start=y_start;
    mmp.z_start=z_start;
    mmp.x_end=x_end;
    mmp.y_end=y_end;
    mmp.z_end=z_end;
    for (int t = 0; t < nt; ++t)
    {
        ptr_t a1 = buffer[(t + 1) % 2];
        cptr_t a0 = buffer[t % 2];
        double pre0[ls], pre1[ls];
        if (p_id != 0)
        {
            //printf("t=%d,进程%d向进程%d发送消息:start.\n", t, p_id, p_id - 1);
            MPI_Isend(a0 + INDEX(0, 0, z_start, ldx, ldy), ls, MPI_DOUBLE, p_id - 1, t, MPI_COMM_WORLD, &request);
            //printf("t=%d,进程%d向进程%d发送消息:done.\n", t, p_id, p_id - 1);
        }

        if (p_id != grid_info->p_num - 1)
        {
            //printf("t=%d,进程%d向进程%d发送消息:start.\n", t, p_id, p_id + 1);
            MPI_Isend(a0 + INDEX(0, 0, z_end - 1, ldx, ldy), ls, MPI_DOUBLE, p_id + 1, t, MPI_COMM_WORLD, &request);
            //printf("t=%d,进程%d向进程%d发送消息:done.\n", t, p_id, p_id + 1);
        }

        mmp.a0 = a0;
        mmp.a1 = a1;

        athread_init();
        athread_spawn(funcee, 0);
        athread_join();
        athread_halt();

        if (p_id != 0)
        {
            //printf("t=%d,进程%d试图接受来自进程%d的消息:start.\n", t, p_id, p_id - 1);
            //MPI_Recv(pre0, ls, MPI_DOUBLE, p_id - 1, t, MPI_COMM_WORLD, &status);
            MPI_Recv(a0 + INDEX(0, 0, z_start - 1, ldx, ldy), ls, MPI_DOUBLE, p_id - 1, t, MPI_COMM_WORLD, &status);
            //printf("t=%d,进程%d试图接受来自进程%d的消息:done.\n", t, p_id, p_id - 1);
        }

        if (p_id != grid_info->p_num - 1)
        {
            //printf("t=%d,进程%d试图接受来自进程%d的消息:start.\n", t, p_id, p_id + 1);
            //MPI_Recv(pre1, ls, MPI_DOUBLE, p_id + 1, t, MPI_COMM_WORLD, &status);
            MPI_Recv(a0 + INDEX(0, 0, z_end, ldx, ldy), ls, MPI_DOUBLE, p_id + 1, t, MPI_COMM_WORLD, &status);
            //printf("t=%d,进程%d试图接受来自进程%d的消息:done.\n", t, p_id, p_id + 1);
        }

        int z = z_start;
        for (int y = y_start; y < y_end; ++y)
        {

            for (int x = x_start; x < x_end; ++x)
            {
                a1[INDEX(x, y, z, ldx, ldy)] = ALPHA_ZZZ * a0[INDEX(x, y, z, ldx, ldy)] + ALPHA_ZZP * a0[INDEX(x, y, z + 1, ldx, ldy)] + ALPHA_NZZ * a0[INDEX(x - 1, y, z, ldx, ldy)] + ALPHA_PZZ * a0[INDEX(x + 1, y, z, ldx, ldy)] + ALPHA_ZNZ * a0[INDEX(x, y - 1, z, ldx, ldy)] + ALPHA_ZPZ * a0[INDEX(x, y + 1, z, ldx, ldy)];
                // if (p_id != 0)
                // {
                //     a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZN * pre0[INDEX(x, y, 0, ldx, ldy)];
                // }
                // else
                // {
                //     a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZN * a0[INDEX(x, y, z - 1, ldx, ldy)];
                // }
                a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZN * a0[INDEX(x, y, z - 1, ldx, ldy)];
            }
        }

        z = z_end - 1;
        for (int y = y_start; y < y_end; ++y)
        {

            for (int x = x_start; x < x_end; ++x)
            {
                a1[INDEX(x, y, z, ldx, ldy)] = ALPHA_ZZN * a0[INDEX(x, y, z - 1, ldx, ldy)] + ALPHA_ZZZ * a0[INDEX(x, y, z, ldx, ldy)] + ALPHA_NZZ * a0[INDEX(x - 1, y, z, ldx, ldy)] + ALPHA_PZZ * a0[INDEX(x + 1, y, z, ldx, ldy)] + ALPHA_ZNZ * a0[INDEX(x, y - 1, z, ldx, ldy)] + ALPHA_ZPZ * a0[INDEX(x, y + 1, z, ldx, ldy)];
                // if (p_id != grid_info->p_num - 1)
                // {
                //     a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZP * pre1[INDEX(x, y, 0, ldx, ldy)];
                // }
                // else
                // {
                //     a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZP * a0[INDEX(x, y, z + 1, ldx, ldy)];
                // }
                a1[INDEX(x, y, z, ldx, ldy)] += ALPHA_ZZP * a0[INDEX(x, y, z + 1, ldx, ldy)];
            }
        }
    }
    return buffer[nt % 2];
}

/* your implementation */
ptr_t stencil_27(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt)
{
    return grid;
}