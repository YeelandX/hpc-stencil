#include <mpi.h>
#include <stdio.h>
#include <stdarg.h>
#include "common.h"

void halo_comm_yz_sync(const dist_grid_info_t *grid_info, ptr_t a0)
{
  int lldz = grid_info->local_size_z;

  int ldx = grid_info->local_size_x + 2 * grid_info->halo_size_x;
  int ldy = grid_info->local_size_y + 2 * grid_info->halo_size_y;
  int ldz = grid_info->local_size_z + 2 * grid_info->halo_size_z;

  int yz_num = (int)sqrt(grid_info->p_num);
  int row_id = grid_info->p_id / yz_num;
  int col_id = grid_info->p_id % yz_num;
  //pack
  int fb_exchange_count = ldx * lldz;
  int ud_exchange_count = ldx * ldy;

  ptr_t up_send = &a0[INDEX(0, 0, grid_info->halo_size_z, ldx, ldy)];
  ptr_t up_recv = &a0[INDEX(0, 0, 0, ldx, ldy)];
  ptr_t down_send = &a0[INDEX(0, 0, grid_info->local_size_z, ldx, ldy)];
  ptr_t down_recv = &a0[INDEX(0, 0, grid_info->local_size_z + grid_info->halo_size_z, ldx, ldy)];

  ptr_t front_send = malloc(4 * fb_exchange_count * sizeof(data_t));
  ptr_t front_recv = front_send + fb_exchange_count;
  ptr_t back_send = front_recv + fb_exchange_count;
  ptr_t back_recv = back_send + fb_exchange_count;
  //comm
  MPI_Status status;
  //right
  int front = col_id == 0 ? MPI_PROC_NULL : grid_info->p_id - 1;//列ID为0进程的front为空
  int back = col_id == yz_num - 1 ? MPI_PROC_NULL : grid_info->p_id + 1;//列ID为yz_num-1的后为空
  int up = row_id == 0 ? MPI_PROC_NULL : grid_info->p_id - yz_num;//行ID为0进程的上为空
  int down = row_id == yz_num - 1 ? MPI_PROC_NULL : grid_info->p_id + yz_num;//列ID为yz_num-1的下为空

  int copy_size = ldx * sizeof(data_t);
  // fb pack
  if (col_id > 0)
    for (int z = 0; z < lldz; ++z)
      memcpy(&front_send[INDEX2(0, z, ldx)], &a0[INDEX(0, grid_info->halo_size_y, z + 1, ldx, ldy)], copy_size);
  if (col_id < yz_num - 1)
    for (int z = 0; z < ldz; ++z)
      memcpy(&back_send[INDEX2(0, z, ldx)], &a0[INDEX(0, grid_info->local_size_y, z + 1, ldx, ldy)], copy_size);

  // fb comm
  MPI_Sendrecv(back_send, fb_exchange_count, DATA_TYPE, back, 0,
               front_recv, fb_exchange_count, DATA_TYPE, front, 0,
               MPI_COMM_WORLD, &status);
  MPI_Sendrecv(front_send, fb_exchange_count, DATA_TYPE, front, 0,
               back_recv, fb_exchange_count, DATA_TYPE, back, 0,
               MPI_COMM_WORLD, &status);
  //fb unpack
  if (col_id < yz_num - 1)
    for (int z = 0; z < lldz; z++)
      memcpy(&a0[INDEX(0, grid_info->local_size_y + grid_info->halo_size_y, z + 1, ldx, ldy)], &back_recv[INDEX2(0, z, ldx)], copy_size);
  if (col_id > 0)
    for (int z = 0; z < lldz; z++)
      memcpy(&a0[INDEX(0, 0, z + 1, ldx, ldy)], &front_recv[INDEX2(0, z, ldx)], copy_size);


  MPI_Sendrecv(down_send, ud_exchange_count, DATA_TYPE, down, 0,
               up_recv, ud_exchange_count, DATA_TYPE, up, 0,
               MPI_COMM_WORLD, &status);
  MPI_Sendrecv(up_send, ud_exchange_count, DATA_TYPE, up, 0,
               down_recv, ud_exchange_count, DATA_TYPE, down, 0,
               MPI_COMM_WORLD, &status);
}