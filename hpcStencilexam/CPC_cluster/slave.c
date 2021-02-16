#include "common.h"
#include "athread.h"
extern mm_param mmp;
extern ldx, ldy, ldz;
extern x_start, y_start, z_start, x_end, y_end, z_end;
#define I 18
#define PE_MODE 1
void funcee()
{
    volatile int i, j, k, my_id, get_reply, put_reply, x, y, z, px, py, pz;
    my_id = athread_get_id(-1);

    double a0_slave[I][I][I], a1_slave[I][I][I];
    int ath_x = mmp.x_start, ath_y = mmp.y_start, ath_z = mmp.z_start;

    for (px = ath_x; px < ath_x + 128; px += 16)
    {
        for (py = ath_y; py < ath_y + 128; py += 16)
        {
            for (pz = ath_z; pz < ath_z + 32; pz += 16)
            {
                get_reply = 0;

                for (j = pz - 1; j <= pz + 16; j++)
                {
                    for (i = py - 1; i <= py + 16; i++)
                    {
                        athread_get(PE_MODE, &mmp.a0[INDEX(px - 1, i, j, mmp.ldx, mmp.ldy)], &a0_slave[0][i - py + 1][j - pz + 1], 18, &get_reply, 0, 0, 0);
                    }
                }
                while (get_reply != 18 * 18)
                {
                    /* code */
                }
                put_reply = 0;
                for (z = 1; z <= 16; z++)
                {
                    for (y = 1; y <= 16; y++)
                    {
                        for (x = 1; x <= 16; x++)
                        {
                            a1_slave[x][y][z] = ALPHA_ZZZ * a0_slave[x][y][z] + ALPHA_NZZ * a0_slave[x - 1][y][z] + ALPHA_PZZ * a0_slave[x + 1][y][z] + ALPHA_ZNZ * a0_slave[x][y - 1][z] + ALPHA_ZPZ * a0_slave[x][y + 1][z] + ALPHA_ZZN * a0_slave[x][y][z - 1] + ALPHA_ZZP * a0_slave[x][y][z + 1];
                        }
                        athread_put(PE_MODE, &a1_slave[1][y + py - 1][z + pz - 1], &mmp.a1[INDEX(px, i, j, mmp.ldx, mmp.ldy), 18, &put_reply, 0, 0]);
                    }
                }
                while (put_reply != 16 * 16)
                {
                    /* code */
                }
            }
        }
    }
}