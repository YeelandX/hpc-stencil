#include <slave.h>
#include "comp_utils.h"
#include "slave_comm.h"
#include "common.h"

#define THREAD_BLOCK_X 512
#define THREAD_HALO 1
#define THREAD_PADDING 3
#define THREAD_LD_X (THREAD_BLOCK_X + 2 * THREAD_HALO + 2 * THREAD_PADDING)

__thread_local int id, sid, next_id, prev_id, ldx, ldy,
    local_x_size, local_y_size, local_z_size, local_x_start, local_y_start,
    local_x_end, local_y_end, local_z_start, local_z_end;

__thread_local ptr_t a1;
__thread_local cptr_t a0;

__thread_local ptr_t i, o, f, b;

__thread_local volatile unsigned long get_reply, put_reply, expect_get;

void reg_sync(int sync_z, int ele_size)
{
  if (sid & 1)
  {
    s_sync(id, prev_id);
    send_to_d(id, prev_id, &i[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
    recv_from_d(id, prev_id, &f[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
  }
  else
  {
    s_sync(id, next_id);
    recv_from_d(id, next_id, &b[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
    send_to_d(id, next_id, &i[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
  }
  if (sid & 1)
  {
    s_sync(id, next_id);
    recv_from_d(id, next_id, &b[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
    send_to_d(id, next_id, &i[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
  }
  else
  {
    s_sync(id, prev_id);
    send_to_d(id, prev_id, &i[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
    recv_from_d(id, prev_id, &f[INDEX2(0, sync_z, THREAD_LD_X)], ele_size);
  }
}

void athread_comp_y_27(const comp_param_t *param)
{
  data_t __attribute__((aligned(32))) ii[3 * THREAD_LD_X],
      __attribute__((aligned(32))) ff[3 * THREAD_LD_X],
      __attribute__((aligned(32))) bb[3 * THREAD_LD_X],
      __attribute__((aligned(32))) oo[THREAD_BLOCK_X],
      __attribute__((aligned(32))) tmp[THREAD_BLOCK_X];

  i = ii;
  f = ff;
  b = bb;
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

  id = athread_get_id(-1);
  sid = s_id(id);
  next_id = s_next_id(id);
  prev_id = s_prev_id(id);

  int y_iter_times = (local_y_size + 63) / 64;
  int y_iter;
  for (y_iter = 0; y_iter < y_iter_times; ++y_iter)
  {
    int y_low = CROSS_BLOCK_LOW(y_iter, y_iter_times, local_y_size);
    int y_high = CROSS_BLOCK_HIGH(y_iter, y_iter_times, local_y_size);
    int y_size = CROSS_BLOCK_SIZE(y_iter, y_iter_times, local_y_size);
    if (sid >= y_size)
      break;
    if (sid + 1 >= y_size)
      next_id = -1;
    int y = local_y_start + y_low + sid;
    int block_x_start;
    for (block_x_start = local_x_start; block_x_start < local_x_end; block_x_start += THREAD_BLOCK_X)
    {
      int block_x_end = min(block_x_start + THREAD_BLOCK_X, local_x_end);
      int block_x_len = block_x_end - block_x_start;
      int get_len = (block_x_len + 2 * THREAD_HALO) * sizeof(data_t);
      int put_len = block_x_len * sizeof(data_t);
      int fb_len = get_len;
      int reg_comm_len = block_x_len + 2 * THREAD_HALO + 2 * THREAD_PADDING;

      int z, zz;
      for (z = local_z_start; z < local_z_end; ++z)
      {
        int cur_z = 1, prev_z = 0, next_z = 2;
        int o_cur_z = 0;
        // 取三层
        get_reply = 0;
        expect_get = 3;
        for (zz = 0; zz < 3; ++zz)
        {
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y, z + zz - 1, ldx, ldy)],
                      &i[INDEX2(THREAD_PADDING, zz, THREAD_LD_X)],
                      get_len, &get_reply, 0, 0, 0);
        }
        // 0 和 63 取前三层  f b
        if (sid == 0)
        {
          expect_get += 3;
          for (zz = 0; zz < 3; ++zz)
          {
            athread_get(PE_MODE,
                        &a0[INDEX(block_x_start - 1, y - 1, z + zz - 1, ldx, ldy)],
                        &f[INDEX2(THREAD_PADDING, zz, THREAD_LD_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
        }
        if (sid == y_size - 1)
        {
          expect_get += 3;
          for (zz = 0; zz < 3; ++zz)
          {
            athread_get(PE_MODE,
                        &a0[INDEX(block_x_start - 1, y + 1, z + zz - 1, ldx, ldy)],
                        &b[INDEX2(THREAD_PADDING, zz, THREAD_LD_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
        }
        while (get_reply != expect_get)
          ;
        // reg 通信
        reg_sync(prev_z, reg_comm_len);
        reg_sync(cur_z, reg_comm_len);
        reg_sync(next_z, reg_comm_len);

        // int ox, ix;
        // for (ox = 0; ox < block_x_len; ++ox)
        // {
        //   ix = ox + THREAD_PADDING + THREAD_HALO;
        //   o[INDEX2(ox, o_cur_z, THREAD_BLOCK_X)] =
        //       ALPHA_ZZZ * i[INDEX2(ix, cur_z, THREAD_LD_X)] +
        //       ALPHA_NZZ * i[INDEX2(ix - 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_PZZ * i[INDEX2(ix + 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_ZZN * i[INDEX2(ix, prev_z, THREAD_LD_X)] +
        //       ALPHA_ZZP * i[INDEX2(ix, next_z, THREAD_LD_X)] +
        //       ALPHA_ZNZ * f[INDEX2(ix, cur_z, THREAD_LD_X)] +
        //       ALPHA_ZPZ * b[INDEX2(ix, cur_z, THREAD_LD_X)] +
        //       ALPHA_NNZ * f[INDEX2(ix - 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_PNZ * f[INDEX2(ix + 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_NPZ * b[INDEX2(ix - 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_PPZ * b[INDEX2(ix + 1, cur_z, THREAD_LD_X)] +
        //       ALPHA_NZN * i[INDEX2(ix - 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_PZN * i[INDEX2(ix + 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_NZP * i[INDEX2(ix - 1, next_z, THREAD_LD_X)] +
        //       ALPHA_PZP * i[INDEX2(ix + 1, next_z, THREAD_LD_X)] +
        //       ALPHA_ZNN * f[INDEX2(ix, prev_z, THREAD_LD_X)] +
        //       ALPHA_ZPN * b[INDEX2(ix, prev_z, THREAD_LD_X)] +
        //       ALPHA_ZNP * f[INDEX2(ix, next_z, THREAD_LD_X)] +
        //       ALPHA_ZPP * b[INDEX2(ix, next_z, THREAD_LD_X)] +
        //       ALPHA_NNN * f[INDEX2(ix - 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_PNN * f[INDEX2(ix + 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_NPN * b[INDEX2(ix - 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_PPN * b[INDEX2(ix + 1, prev_z, THREAD_LD_X)] +
        //       ALPHA_NNP * f[INDEX2(ix - 1, next_z, THREAD_LD_X)] +
        //       ALPHA_PNP * f[INDEX2(ix + 1, next_z, THREAD_LD_X)] +
        //       ALPHA_NPP * b[INDEX2(ix - 1, next_z, THREAD_LD_X)] +
        //       ALPHA_PPP * b[INDEX2(ix + 1, next_z, THREAD_LD_X)];
        // }
        // comp
        vector_vmuld_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZZZ);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZZ);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZZ);
        vector_vmad_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZZN);
        vector_vmad_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZZP);
        vector_vmad_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZNZ);
        vector_vmad_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZPZ);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNZ);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNZ);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPZ);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPZ);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZN);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZN);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZP);
        vector_vmad_u_32(o, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZP);
        vector_vmad_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZNN);
        vector_vmad_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZPN);
        vector_vmad_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZNP);
        vector_vmad_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZPP);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNN);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNN);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPN);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPN);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNP);
        vector_vmad_u_32(o, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNP);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPP);
        vector_vmad_u_32(o, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPP);

        put_reply = 0;
        athread_put(PE_MODE, &o[INDEX2(0, o_cur_z, THREAD_BLOCK_X)],
                    &a1[INDEX(block_x_start, y, z, ldx, ldy)],
                    put_len, &put_reply, 0, 0);
        while (put_reply != 1)
          ;
      }
    }
  }
}