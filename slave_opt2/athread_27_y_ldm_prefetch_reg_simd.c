#include <slave.h>
#include "comp_utils.h"
#include "slave_comm.h"
#include "common.h"

#define THREAD_BLOCK_X 256
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

void reg_sync_2(int sync_z, int ele_size)
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

void athread_comp_y_27_prefetch(const comp_param_t *param)
{
  data_t __attribute__((aligned(32))) ii[4 * THREAD_LD_X],
      __attribute__((aligned(32))) ff[4 * THREAD_LD_X],
      __attribute__((aligned(32))) bb[4 * THREAD_LD_X],
      __attribute__((aligned(32))) oo[2 * THREAD_BLOCK_X],
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
      // 取前三层
      get_reply = 0;
      expect_get = 3;
      for (z = 0; z < 3; ++z)
      {
        zz = local_z_start + z - 1;
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start - 1, y, zz, ldx, ldy)],
                    &i[INDEX2(THREAD_PADDING, zz & 3, THREAD_LD_X)],
                    get_len, &get_reply, 0, 0, 0);
      }
      while (get_reply != expect_get)
        ;

      // 0 和 63 取前三层  f b
      if (sid == 0)
      {
        expect_get += 3;
        for (z = 0; z < 3; ++z)
        {
          zz = local_z_start + z - 1;
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y - 1, zz, ldx, ldy)],
                      &f[INDEX2(THREAD_PADDING, zz & 3, THREAD_LD_X)],
                      fb_len, &get_reply, 0, 0, 0);
        }
      }
      if (sid == y_size - 1)
      {
        expect_get += 3;
        for (z = 0; z < 3; ++z)
        {
          zz = local_z_start + z - 1;
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y + 1, zz, ldx, ldy)],
                      &b[INDEX2(THREAD_PADDING, zz & 3, THREAD_LD_X)],
                      fb_len, &get_reply, 0, 0, 0);
        }
      }
      // reg 通信 前两层
      reg_sync_2((local_z_start + 3) & 3, reg_comm_len);
      reg_sync_2(local_z_start & 3, reg_comm_len);

      while (get_reply != expect_get)
        ;

      put_reply = 1;
      for (z = local_z_start; z < local_z_end; ++z)
      {
        // prefetch
        get_reply = 0;
        expect_get = 1;
        int z_prefetch = z + 2;
        if (z_prefetch <= local_z_end)
        {
          athread_get(PE_MODE,
                      &a0[INDEX(block_x_start - 1, y, z_prefetch, ldx, ldy)],
                      &i[INDEX2(THREAD_PADDING, z_prefetch & 3, THREAD_LD_X)],
                      get_len, &get_reply, 0, 0, 0);

          if (sid == 0)
          {
            expect_get += 1;
            athread_get(PE_MODE,
                        &a0[INDEX(block_x_start - 1, y - 1, z_prefetch, ldx, ldy)],
                        &f[INDEX2(THREAD_PADDING, z_prefetch & 3, THREAD_LD_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
          if (sid == y_size - 1)
          {
            expect_get += 1;
            athread_get(PE_MODE,
                        &a0[INDEX(block_x_start - 1, y + 1, z_prefetch, ldx, ldy)],
                        &b[INDEX2(THREAD_PADDING, z_prefetch & 3, THREAD_LD_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
        }

        int cur_z = z & 3, prev_z = (z + 3) & 3, next_z = (z + 1) & 3;
        int o_cur_z = z & 1;
        ptr_t o_cur = &o[INDEX2(0, o_cur_z, THREAD_BLOCK_X)];
        // reg 通信第三层
        reg_sync_2(next_z, reg_comm_len);

        // comp
        vector_vmuld_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZZZ);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZZ);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZZ);
        vector_vmad_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZZN);
        vector_vmad_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZZP);
        vector_vmad_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZNZ);
        vector_vmad_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len, ALPHA_ZPZ);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNZ);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNZ);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPZ);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, cur_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPZ);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZN);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZN);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NZP);
        vector_vmad_u_32(o_cur, &i[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PZP);
        vector_vmad_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZNN);
        vector_vmad_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO, prev_z, THREAD_LD_X)], block_x_len, ALPHA_ZPN);
        vector_vmad_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZNP);
        vector_vmad_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO, next_z, THREAD_LD_X)], block_x_len, ALPHA_ZPP);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNN);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNN);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPN);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, prev_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPN);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NNP);
        vector_vmad_u_32(o_cur, &f[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PNP);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO - 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_NPP);
        vector_vmad_u_32(o_cur, &b[INDEX2(THREAD_PADDING + THREAD_HALO + 1, next_z, THREAD_LD_X)], tmp, block_x_len, ALPHA_PPP);

        //等待上层写完
        while (put_reply != 1)
          ;
        //写出当前层
        put_reply = 0;
        athread_put(PE_MODE, &o[INDEX2(0, o_cur_z, THREAD_BLOCK_X)],
                    &a1[INDEX(block_x_start, y, z, ldx, ldy)],
                    put_len, &put_reply, 0, 0);
        //等待预取完成
        if (z_prefetch <= local_z_end)
        {
          while (get_reply != expect_get)
            ;
        }
      }
      while (put_reply != 1)
        ;
    }
  }
}