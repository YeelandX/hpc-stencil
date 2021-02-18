#include <slave.h>
#include "slave_comm.h"
#include "common.h"

#define THREAD_BLOCK_X 768
#define THREAD_HALO 1
#define THREAD_PADDING 3

#define THREAD_LD_X (THREAD_BLOCK_X + 2 * THREAD_HALO + 2 * THREAD_PADDING)

__thread_local int  id, sid, next_id, prev_id, ldx, ldy,
    local_x_size, local_y_size, local_z_size, local_x_start, local_y_start,
    local_x_end, local_y_end, local_z_start, local_z_end;

__thread_local ptr_t a1;
__thread_local cptr_t a0;

__thread_local volatile unsigned long get_reply, put_reply, expect_get;

void athread_halo_comp_z_7(const comp_param_t *param)
{

  data_t __attribute__((aligned(32))) i[THREAD_LD_X],
      __attribute__((aligned(32))) f[THREAD_BLOCK_X],
      __attribute__((aligned(32))) b[THREAD_BLOCK_X],
      __attribute__((aligned(32))) u[THREAD_BLOCK_X],
      __attribute__((aligned(32))) d[THREAD_BLOCK_X],
      __attribute__((aligned(32))) o[THREAD_BLOCK_X];

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
      int fb_len = block_x_len * sizeof(data_t);
      int ud_len = block_x_len * sizeof(data_t);
      // 读取 i
      get_reply = 0;
      expect_get = 1;
      athread_get(PE_MODE,
                  &a0[INDEX(block_x_start - 1, y, z, ldx, ldy)],
                  &i[THREAD_PADDING],
                  get_len, &get_reply, 0, 0, 0);
      while (get_reply != expect_get)
        ;
      // 获取 front
      expect_get += 2;
      athread_get(PE_MODE,
                  &a0[INDEX(block_x_start, y - 1, z, ldx, ldy)],
                  f, fb_len, &get_reply, 0, 0, 0);
      // 获取 end
      athread_get(PE_MODE,
                  &a0[INDEX(block_x_start, y + 1, z, ldx, ldy)],
                  b, fb_len, &get_reply, 0, 0, 0);

      // 获取 up
      if (sid == 0)
      {
        expect_get += 1;
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start, y, z - 1, ldx, ldy)],
                    u, ud_len, &get_reply, 0, 0, 0);
      }
      // 获取 down
      if (sid == z_size - 1)
      {
        expect_get += 1;
        athread_get(PE_MODE,
                    &a0[INDEX(block_x_start, y, z + 1, ldx, ldy)],
                    d, ud_len, &get_reply, 0, 0, 0);
      }
      // reg comm
      if (sid & 1)
      {
        s_sync(id, prev_id);
        send_to_d(id, prev_id, &i[THREAD_PADDING + THREAD_HALO], block_x_len);
        recv_from_d(id, prev_id, u, block_x_len);
      }
      else
      {
        s_sync(id, next_id);
        recv_from_d(id, next_id, d, block_x_len);
        send_to_d(id, next_id, &i[THREAD_PADDING + THREAD_HALO], block_x_len);
      }
      if (sid & 1)
      {
        s_sync(id, next_id);
        recv_from_d(id, next_id, d, block_x_len);
        send_to_d(id, next_id, &i[THREAD_PADDING + THREAD_HALO], block_x_len);
      }
      else
      {
        s_sync(id, prev_id);
        send_to_d(id, prev_id, &i[THREAD_PADDING + THREAD_HALO], block_x_len);
        recv_from_d(id, prev_id, u, block_x_len);
      }

      while (get_reply != expect_get)
        ;

      int ox, ix;
      for (ox = 0; ox < block_x_len; ++ox)
      {
        ix = ox + THREAD_PADDING + THREAD_HALO;
        o[ox] =
            ALPHA_ZZZ * i[ix] +
            ALPHA_NZZ * i[ix - 1] +
            ALPHA_PZZ * i[ix + 1] +
            ALPHA_ZZN * u[ox] +
            ALPHA_ZZP * d[ox] +
            ALPHA_ZNZ * f[ox] +
            ALPHA_ZPZ * b[ox];
      }
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