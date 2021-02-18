#include <slave.h>
#include "comp_utils.h"
#include "slave_comm.h"
#include "common.h"

#define THREAD_BLOCK_X 512
#define THREAD_HALO 1
#define THREAD_PADDING 3

#define THREAD_LD_X (THREAD_BLOCK_X + 2 * THREAD_HALO + 2 * THREAD_PADDING)

__thread_local int id, prev_id, next_id, sid, ldx, ldy,
    local_x_size, local_y_size, local_z_size, local_x_start, local_y_start,
    local_x_end, local_y_end, local_z_start, local_z_end;

__thread_local ptr_t a1;
__thread_local cptr_t a0;

__thread_local ptr_t i, u, d, o;

__thread_local volatile unsigned long get_reply, put_reply, expect_get;

void reg_sync_3(int sync_y, int ele_size)
{
  if (sid & 1)
  {
    s_sync(id, prev_id);
    send_to_d(id, prev_id, &i[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
    recv_from_d(id, prev_id, &u[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
  }
  else
  {
    s_sync(id, next_id);
    recv_from_d(id, next_id, &d[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
    send_to_d(id, next_id, &i[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
  }
  if (sid & 1)
  {
    s_sync(id, next_id);
    recv_from_d(id, next_id, &d[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
    send_to_d(id, next_id, &i[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
  }
  else
  {
    s_sync(id, prev_id);
    send_to_d(id, prev_id, &i[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
    recv_from_d(id, prev_id, &u[INDEX2(0, sync_y, THREAD_LD_X)], ele_size);
  }
}

void athread_halo_comp_z_27(const comp_param_t *param)
{

  data_t __attribute__((aligned(32))) ii[3 * THREAD_LD_X],
      __attribute__((aligned(32))) uu[3 * THREAD_LD_X],
      __attribute__((aligned(32))) dd[3 * THREAD_LD_X],
      __attribute__((aligned(32))) oo[THREAD_BLOCK_X],
      __attribute__((aligned(32))) tmp[THREAD_BLOCK_X];

  i = ii;
  u = uu;
  d = dd;
  o = oo;
  a1 = param->a1;
  a0 = param->a0;
  ldx = param->ldx;
  ldy = param->ldy;
  local_x_size = param->x_size;
  local_y_size = param->y_size;
  local_z_size = param->z_size;
  local_x_start = param->x_start;
  local_y_start = param->y_start;
  local_z_start = param->z_start;
  local_x_end = local_x_start + local_x_size;
  local_y_end = local_y_start + local_y_size;
  local_z_end = local_z_start + local_z_size;

  if (local_y_size != 1)
  {
    printf("in athread_halo_comp_z_7 local_y_size should be 1 but %d\n", local_y_size);
    exit(-1);
  }

  id = athread_get_id(-1);
  sid = s_id(id);
  next_id = s_next_id(id);
  prev_id = s_prev_id(id);

  int y = local_y_start;
  int z_iter_times = (local_z_size + 63) / 64;
  int z_iter;
  for (z_iter = 0; z_iter < z_iter_times; ++z_iter)
  {
    int z_low = CROSS_BLOCK_LOW(z_iter, z_iter_times, local_z_size);
    int z_size = CROSS_BLOCK_SIZE(z_iter, z_iter_times, local_z_size);
    if (sid >= z_size)
      break;
    if (sid + 1 >= z_size)
      next_id = -1;
    int z = local_z_start + z_low + sid;
    int block_x_start;
    for (block_x_start = local_x_start; block_x_start < local_x_end; block_x_start += THREAD_BLOCK_X)
    {
      int block_x_end = min(block_x_start + THREAD_BLOCK_X, local_x_end);
      int block_x_len = block_x_end - block_x_start;
      int get_len = (block_x_len + 2 * THREAD_HALO) * sizeof(data_t);
      int put_len = block_x_len * sizeof(data_t);
      int ud_len = get_len;
      int reg_comm_len = block_x_len + 2 * THREAD_HALO + 2 * THREAD_PADDING;

      get_reply = 0;
      expect_get = 3;
      int yy;
      for (yy = 0; yy < 3; ++yy)
      {
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start - 1, y + yy - 1, z, ldx, ldy)],
                    &i[INDEX2(THREAD_PADDING, yy, THREAD_LD_X)],
                    get_len, &get_reply, 0, 0, 0);
      }
      while (get_reply != expect_get)
        ;
      // 获取 up
      if (sid == 0)
      {
        expect_get += 3;
        for (yy = 0; yy < 3; ++yy)
        {
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y + yy - 1, z - 1, ldx, ldy)],
                      &u[INDEX2(THREAD_PADDING, yy, THREAD_LD_X)],
                      ud_len, &get_reply, 0, 0, 0);
        }
      }
      // 获取 down
      if (sid == z_size - 1)
      {
        expect_get += 3;
        for (yy = 0; yy < 3; ++yy)
        {
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y + yy - 1, z + 1, ldx, ldy)],
                      &d[INDEX2(THREAD_PADDING, yy, THREAD_LD_X)],
                      ud_len, &get_reply, 0, 0, 0);
        }
      }

      int prev_y = 0, cur_y = 1, next_y = 2;
      reg_sync_3(prev_y, reg_comm_len);
      reg_sync_3(cur_y, reg_comm_len);
      reg_sync_3(next_y, reg_comm_len);

      while (get_reply != expect_get)
        ;
      vector_vmuld_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_y, THREAD_LD_X)], block_x_len, ALPHA_ZZZ);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZZ);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZZ);
      vector_vmad_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, prev_y, THREAD_LD_X)], block_x_len, ALPHA_ZNZ);
      vector_vmad_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, next_y, THREAD_LD_X)], block_x_len, ALPHA_ZPZ);
      vector_vmad_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO, cur_y, THREAD_LD_X)], block_x_len, ALPHA_ZZN);
      vector_vmad_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO, cur_y, THREAD_LD_X)], block_x_len, ALPHA_ZZP);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNZ);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNZ);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPZ);
      vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPZ);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZN);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZN);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZP);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZP);
      vector_vmad_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO, prev_y, THREAD_LD_X)], block_x_len, ALPHA_ZNN);
      vector_vmad_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO, next_y, THREAD_LD_X)], block_x_len, ALPHA_ZPN);
      vector_vmad_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO, prev_y, THREAD_LD_X)], block_x_len, ALPHA_ZNP);
      vector_vmad_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO, next_y, THREAD_LD_X)], block_x_len, ALPHA_ZPP);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNN);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNN);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPN);
      vector_vmad_u_32(o, &u[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPN);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNP);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNP);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPP);
      vector_vmad_u_32(o, &d[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_y, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPP);

      //写出当前层
      put_reply = 0;
      athread_put(PE_MODE, o,
                  &a1[INDEX(block_x_start, y, z, ldx, ldy)],
                  put_len, &put_reply, 0, 0);
      while (put_reply != 1)
        ;
    }
  }
}