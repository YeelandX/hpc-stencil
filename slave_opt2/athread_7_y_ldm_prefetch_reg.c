#include <slave.h>
#include "slave_comm.h"
#include "common.h"

#define THREAD_BLOCK_X 768
#define THREAD_HALO 1
#define THREAD_PADDING 3
#define THREAD_LD_X (THREAD_BLOCK_X + 2 * THREAD_HALO + 2 * THREAD_PADDING)

__thread_local int id, sid, next_id, prev_id, ldx, ldy,
    local_x_size, local_y_size, local_z_size, local_x_start, local_y_start,
    local_x_end, local_y_end, local_z_start, local_z_end;

__thread_local int get_len, put_len, fb_len;

__thread_local ptr_t a1;
__thread_local cptr_t a0;

__thread_local volatile unsigned long get_reply, put_reply, expect_get;

void athread_comp_y_7_prefetch(const comp_param_t *param)
{
  data_t __attribute__((aligned(32))) i[4 * THREAD_LD_X],
      __attribute__((aligned(32))) f[2 * THREAD_BLOCK_X],
      __attribute__((aligned(32))) b[2 * THREAD_BLOCK_X],
      __attribute__((aligned(32))) o[2 * THREAD_BLOCK_X];

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

  int y_iter, y, block_x_start;
  int y_iter_times = (local_y_size + 63) / 64;
  for (y_iter = 0; y_iter < y_iter_times; ++y_iter)
  {
    int y_low = CROSS_BLOCK_LOW(y_iter, y_iter_times, local_y_size);
    int y_size = CROSS_BLOCK_SIZE(y_iter, y_iter_times, local_y_size);
    if (sid >= y_size)
      break;
    if (sid + 1 >= y_size)
      next_id = -1;
    y = local_y_start + y_low + sid;
    for (block_x_start = local_x_start; block_x_start < local_x_end; block_x_start += THREAD_BLOCK_X)
    {
      int block_x_end = min(block_x_start + THREAD_BLOCK_X, local_x_end);
      int block_x_len = block_x_end - block_x_start;
      int get_len = (block_x_len + 2 * THREAD_HALO) * sizeof(data_t);
      int put_len = block_x_len * sizeof(data_t);
      int fb_len = block_x_len * sizeof(data_t);
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
      // 0 和 63 取  f b
      if (sid == 0)
      {
        expect_get += 1;
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start, y - 1, local_z_start, ldx, ldy)],
                    &f[INDEX2(0, local_z_start & 1, THREAD_BLOCK_X)],
                    fb_len, &get_reply, 0, 0, 0);
      }
      if (sid == y_size - 1)
      {
        expect_get += 1;
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start, y + 1, local_z_start, ldx, ldy)],
                    &b[INDEX2(0, local_z_start & 1, THREAD_BLOCK_X)],
                    fb_len, &get_reply, 0, 0, 0);
      }
      while (get_reply != expect_get)
        ;
      put_reply = 1;
      for (z = local_z_start; z < local_z_end; ++z)
      {
        // prefetch
        get_reply = 0;
        expect_get = 1;
        int z_prefetch = z + 2, fb_prefetch = z + 1;
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
                        &a0[INDEX(block_x_start, y - 1, fb_prefetch, ldx, ldy)],
                        &f[INDEX2(0, fb_prefetch & 1, THREAD_BLOCK_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
          if (sid == y_size - 1)
          {
            expect_get += 1;
            athread_get(PE_MODE,
                        &a0[INDEX(block_x_start, y + 1, fb_prefetch, ldx, ldy)],
                        &b[INDEX2(0, fb_prefetch & 1, THREAD_BLOCK_X)],
                        fb_len, &get_reply, 0, 0, 0);
          }
        }

        int cur_z = z & 3, prev_z = (z + 3) & 3, next_z = (z + 1) & 3;
        int o_cur_z = z & 1;

        if (sid & 1)//sid为奇数时执行
        {
          s_sync(id, prev_id);
          send_to_d(id, prev_id, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len);
          recv_from_d(id, prev_id, &f[INDEX2(0, o_cur_z, THREAD_BLOCK_X)], block_x_len);
        }
        else
        {
          s_sync(id, next_id);
          recv_from_d(id, next_id, &b[INDEX2(0, o_cur_z, THREAD_BLOCK_X)], block_x_len);
          send_to_d(id, next_id, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len);
        }
        if (sid & 1)
        {
          s_sync(id, next_id);
          recv_from_d(id, next_id, &b[INDEX2(0, o_cur_z, THREAD_BLOCK_X)], block_x_len);
          send_to_d(id, next_id, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len);
        }
        else
        {
          s_sync(id, prev_id);
          send_to_d(id, prev_id, &i[INDEX2(THREAD_PADDING + THREAD_HALO, cur_z, THREAD_LD_X)], block_x_len);
          recv_from_d(id, prev_id, &f[INDEX2(0, o_cur_z, THREAD_BLOCK_X)], block_x_len);
        }

        int ox, ix;
        for (ox = 0; ox < block_x_len; ++ox)
        {
          ix = ox + THREAD_PADDING + THREAD_HALO;
          o[INDEX2(ox, o_cur_z, THREAD_BLOCK_X)] =
              ALPHA_ZZZ * i[INDEX2(ix, cur_z, THREAD_LD_X)] +
              ALPHA_NZZ * i[INDEX2(ix - 1, cur_z, THREAD_LD_X)] +
              ALPHA_PZZ * i[INDEX2(ix + 1, cur_z, THREAD_LD_X)] +
              ALPHA_ZZN * i[INDEX2(ix, prev_z, THREAD_LD_X)] +
              ALPHA_ZZP * i[INDEX2(ix, next_z, THREAD_LD_X)] +
              ALPHA_ZNZ * f[INDEX2(ox, o_cur_z, THREAD_BLOCK_X)] +
              ALPHA_ZPZ * b[INDEX2(ox, o_cur_z, THREAD_BLOCK_X)];
        }

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