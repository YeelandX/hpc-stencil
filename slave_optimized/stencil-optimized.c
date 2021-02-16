#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "common.h"
#include <athread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "simd.h"
#define xy_dim 264196
//add
extern SLAVE_FUN(func_7)();
extern SLAVE_FUN(func_27)();
int ldx, ldy, ldz;
cptr_t a0;
ptr_t a1;

const char *version_name = "Optimized version";

/* your implementation */
void create_dist_grid(dist_grid_info_t *grid_info, int stencil_type)
{

	grid_info->local_size_x = grid_info->global_size_x;
	grid_info->local_size_y = grid_info->global_size_y;
	grid_info->local_size_z = grid_info->global_size_z / 16;

	grid_info->offset_x = 0;
	grid_info->offset_y = 0;
	grid_info->offset_z = grid_info->global_size_z / 16 * grid_info->p_id;
	grid_info->halo_size_x = 1;
	grid_info->halo_size_y = 1;
	grid_info->halo_size_z = 1;
	athread_init();
}

/* your implementation */
void destroy_dist_grid(dist_grid_info_t *grid_info)
{
	athread_halt();
}

/* your implementation */
ptr_t stencil_7(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt)
{
	//unsigned long st, ed,time;
	//unsigned long t1, t2;
	//add master-slave asyn6.9
	MPI_Status status[4];
	MPI_Request request[4];
	ptr_t buffer[2] = {grid, aux};
	ldx = grid_info->local_size_x + 2;
	ldy = grid_info->local_size_y + 2;
	ldz = grid_info->local_size_z + 2;
	//add youhuamsasyn
	//int xy_dim = ldx * ldy;
	int ldz_2 = (ldz - 2) * xy_dim;
	int ldz_1 = (ldz - 1) * xy_dim;
	//add
	//athread_init();
	for (int t = 0; t < nt; ++t)
	{
		a0 = buffer[t % 2];
		a1 = buffer[(t + 1) % 2];

		/*ÿ�ε�������halo��ֵ*/
		if (grid_info->p_id % 2 == 0)
		{
			MPI_Send(&a0[INDEX(0, 0, ldz - 2, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD);
			MPI_Recv(&a0[INDEX(0, 0, ldz - 1, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &status);
		}
		else
		{
			MPI_Recv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &status);
			MPI_Send(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD);
		}
		if (grid_info->p_id != 0 && grid_info->p_id != 15)
		{
			if (grid_info->p_id % 2 == 1)
			{
				MPI_Send(&a0[INDEX(0, 0, ldz - 2, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD);
				MPI_Recv(&a0[INDEX(0, 0, ldz - 1, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &status);
			}
			else
			{
				MPI_Recv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &status);
				MPI_Send(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx * ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD);
			}
		}
		athread_spawn(func_7, 0);
		athread_join();
	}
	return buffer[nt % 2];
}

/* your implementation */
ptr_t stencil_27(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt)
{
	//MPI_Status status;
	MPI_Status status[4];
	MPI_Request request[4];
	ptr_t buffer[2] = {grid, aux};
	/*int x_start = 0, x_end = grid_info->local_size_x + 2 * grid_info->halo_size_x;
	int y_start = 0, y_end = grid_info->local_size_y + 2 * grid_info->halo_size_y;
	int z_start = 0, z_end = grid_info->local_size_z + 2 * grid_info->halo_size_z;*/
	ldx = grid_info->local_size_x + 2;
	ldy = grid_info->local_size_y + 2;
	ldz = grid_info->local_size_z + 2;
	int ldz_2 = (ldz - 2) * xy_dim;
	int ldz_1 = (ldz - 1) * xy_dim;
	for (int t = 0; t < nt; ++t)
	{
		a0 = buffer[t % 2];
		a1 = buffer[(t + 1) % 2];
		/*ÿ�ε�������halo��ֵ*/
		/*if (grid_info->p_id % 2 == 0) {
			MPI_Send(&a0[INDEX(0, 0, z_end - 2, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD);
			MPI_Recv(&a0[INDEX(0, 0, z_end - 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &status);
		}
		else {
			MPI_Recv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &status);
			MPI_Send(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD);
		}
		if (grid_info->p_id != 0 && grid_info->p_id != 15) {
			if (grid_info->p_id % 2 == 1) {
				MPI_Send(&a0[INDEX(0, 0, z_end - 2, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD);
				MPI_Recv(&a0[INDEX(0, 0, z_end - 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &status);
			}
			else {
				MPI_Recv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &status);
				MPI_Send(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD);
			}
		}*/
		if (grid_info->p_id != 0 && grid_info->p_id != 15)
		{
			/*MPI_Isend(&a0[INDEX(0, 0, ldz - 2, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[0]);
			MPI_Irecv(&a0[INDEX(0, 0, ldz - 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[1]);
			MPI_Irecv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[2]);
			MPI_Isend(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[3]);*/
			MPI_Isend(&a0[ldz_2], xy_dim, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[0]); //send to next process
			MPI_Irecv(&a0[0], xy_dim, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[2]);
			MPI_Isend(&a0[xy_dim], xy_dim, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[3]); //sent to front process
			MPI_Irecv(&a0[ldz_1], xy_dim, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[1]);
		}
		else if (grid_info->p_id == 0)
		{
			/*MPI_Isend(&a0[INDEX(0, 0, ldz - 2, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[0]);
			MPI_Irecv(&a0[INDEX(0, 0, ldz - 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[1]);*/
			MPI_Isend(&a0[ldz_2], xy_dim, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[0]);
			MPI_Irecv(&a0[ldz_1], xy_dim, DATA_TYPE, grid_info->p_id + 1, 0, MPI_COMM_WORLD, &request[1]);
		}
		else if (grid_info->p_id == 15)
		{
			/*MPI_Irecv(&a0[INDEX(0, 0, 0, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[0]);
			MPI_Isend(&a0[INDEX(0, 0, 1, ldx, ldy)], ldx*ldy, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[1]);*/
			MPI_Irecv(&a0[0], xy_dim, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[2]);
			MPI_Isend(&a0[xy_dim], xy_dim, DATA_TYPE, grid_info->p_id - 1, 0, MPI_COMM_WORLD, &request[3]);
		}
		/*athread_spawn(func_27, 0);
		athread_join();*/
		/*zhushi if bianyi7*/

		/*if (grid_info->p_id != 0 && grid_info->p_id != 15) {
			MPI_Waitall(4, request, status);
		}
		else if (grid_info->p_id == 0) {
			MPI_Waitall(2, request, status);
		}
		else if (grid_info->p_id == 15) {
			MPI_Waitall(2, request, status);
		}

		ldz = 1;
		athread_spawn(func_27, 0);
		athread_join();

		ldz = grid_info->local_size_z;
		athread_spawn(func_27, 0);
		athread_join();*/
		//add
		if (grid_info->p_id > 0)
		{
			MPI_Wait(&request[2], status);
		}
		ldz = 1;
		/*athread_spawn(func_27, 0);
		athread_join();*/
		/*zhushi if bianyi7*/
		if (grid_info->p_id < 15)
		{
			MPI_Wait(&request[1], status);
		}

		ldz = grid_info->local_size_z;
		/*athread_spawn(func_27, 0);
		athread_join();*/
		/*zhushi if bianyi7*/

		ldz = grid_info->local_size_z + 2;
		for (int z = 1; z < ldz - 1; ++z)
		{
			//set_zero(a1 + INDEX(0, 0, z, ldx, ldy), ldx);
			set_zero(a1 + z * xy_dim, ldx);
			for (int y = 1; y < ldy - 1; ++y)
			{
				/*set_zero(a1 + INDEX(0, y, z, ldx, ldy), 1);
				set_zero(a1 + INDEX(ldx - 1, y, z, ldx, ldy), 1);*/
				a1[y * ldx + z * xy_dim] = 0;
				a1[ldx - 1 + y * ldx + z * xy_dim] = 0;
			}
			//set_zero(a1 + INDEX(0, ldy - 1, z, ldx, ldy), ldx);
			set_zero(a1 + (ldy - 1) * ldx + z * xy_dim, ldx);
		}
		/*for (int z = 1; z < z_end-1; ++z) {
			for (int y = 1; y < y_end-1; ++y) {
				for (int x = 1; x < x_end-1; ++x) {
					a1[INDEX(x, y, z, ldx, ldy)] \
						= ALPHA_ZZZ * a0[INDEX(x, y, z, ldx, ldy)] \
						+ ALPHA_NZZ * a0[INDEX(x - 1, y, z, ldx, ldy)] \
						+ ALPHA_PZZ * a0[INDEX(x + 1, y, z, ldx, ldy)] \
						+ ALPHA_ZNZ * a0[INDEX(x, y - 1, z, ldx, ldy)] \
						+ ALPHA_ZPZ * a0[INDEX(x, y + 1, z, ldx, ldy)] \
						+ ALPHA_ZZN * a0[INDEX(x, y, z - 1, ldx, ldy)] \
						+ ALPHA_ZZP * a0[INDEX(x, y, z + 1, ldx, ldy)] \
						+ ALPHA_NNZ * a0[INDEX(x - 1, y - 1, z, ldx, ldy)] \
						+ ALPHA_PNZ * a0[INDEX(x + 1, y - 1, z, ldx, ldy)] \
						+ ALPHA_NPZ * a0[INDEX(x - 1, y + 1, z, ldx, ldy)] \
						+ ALPHA_PPZ * a0[INDEX(x + 1, y + 1, z, ldx, ldy)] \
						+ ALPHA_NZN * a0[INDEX(x - 1, y, z - 1, ldx, ldy)] \
						+ ALPHA_PZN * a0[INDEX(x + 1, y, z - 1, ldx, ldy)] \
						+ ALPHA_NZP * a0[INDEX(x - 1, y, z + 1, ldx, ldy)] \
						+ ALPHA_PZP * a0[INDEX(x + 1, y, z + 1, ldx, ldy)] \
						+ ALPHA_ZNN * a0[INDEX(x, y - 1, z - 1, ldx, ldy)] \
						+ ALPHA_ZPN * a0[INDEX(x, y + 1, z - 1, ldx, ldy)] \
						+ ALPHA_ZNP * a0[INDEX(x, y - 1, z + 1, ldx, ldy)] \
						+ ALPHA_ZPP * a0[INDEX(x, y + 1, z + 1, ldx, ldy)] \
						+ ALPHA_NNN * a0[INDEX(x - 1, y - 1, z - 1, ldx, ldy)] \
						+ ALPHA_PNN * a0[INDEX(x + 1, y - 1, z - 1, ldx, ldy)] \
						+ ALPHA_NPN * a0[INDEX(x - 1, y + 1, z - 1, ldx, ldy)] \
						+ ALPHA_PPN * a0[INDEX(x + 1, y + 1, z - 1, ldx, ldy)] \
						+ ALPHA_NNP * a0[INDEX(x - 1, y - 1, z + 1, ldx, ldy)] \
						+ ALPHA_PNP * a0[INDEX(x + 1, y - 1, z + 1, ldx, ldy)] \
						+ ALPHA_NPP * a0[INDEX(x - 1, y + 1, z + 1, ldx, ldy)] \
						+ ALPHA_PPP * a0[INDEX(x + 1, y + 1, z + 1, ldx, ldy)];
				}
			}
		}*/
	}
	return buffer[nt % 2];
}