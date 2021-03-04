#include <stdio.h>
#include <math.h>
#include <string.h>
#include <slave.h>
#include "common.h"
#include "dma.h"

#define ldx 514
#define ldy 514
extern cptr_t a0;
extern ptr_t a1;
__thread_local double a0_slave[4][6][130], a0_slave_lr[2][4][128], a1_slave[4][4][128];
__thread_local volatile unsigned long get_reply, put_reply;

// void cal_inner(int x, int y, int z)
// {
//     int i, j, k;
//     // get_reply = 0;
//     // put_reply = 0;
//     get_reply = 0;
//     dma_descriptor_init(&get_desc, get_reply);
//     //get
//     dma_set_reply(&get_desc, &get_reply);
//     dma_set_size(&get_desc, 130 * 8);
//     for (k = z; k < z + 4; k++)
//     {
//         for (j = y - 1; j <= y + 4; j++)
//         {
//             dma(get_desc, &a0[INDEX(x - 1, j, k, ldx, ldy)], &a0_slave[k - z][j - y + 1][0]);
//             //athread_get(PE_MODE, &a0[INDEX(x - 1, j, k, ldx, ldy)], &a0_slave[k - z][j - y + 1][0], 130 * 8, &get_reply, 0, 0, 0);
//         }
//     }

//     //dma_wait(&get_reply, 24);
//     //get_reply=0;
//     //dma_set_reply(&get_desc, &get_reply);
//     dma_set_size(&get_desc, 128 * 8);
//     for (j = y; j < y + 4; j++)
//     {
//         dma(get_desc, &a0[INDEX(x, j, z - 1, ldx, ldy)], &a0_slave_lr[0][j - y][0]);
//         dma(get_desc, &a0[INDEX(x, j, z + 4, ldx, ldy)], &a0_slave_lr[1][j - y][0]);
//         // athread_get(PE_MODE, &a0[INDEX(x, j, z - 1, ldx, ldy)], &a0_slave_lr[0][j - y][0], 128 * 8, &get_reply, 0, 0, 0);
//         // athread_get(PE_MODE, &a0[INDEX(x, j, z + 4, ldx, ldy)], &a0_slave_lr[1][j - y][0], 128 * 8, &get_reply, 0, 0, 0);
//     }

//     //printf("get:%d\n", get_reply);
//     //dma_wait(&get_reply, 8);
//     dma_wait(&get_reply, 32);
//     // while (get_reply != 32) //24+4+4
//     //     ;
//     //printf("get done!");
//     //cal
//     for (k = 1; k < 3; k++)
//     {
//         for (j = 1; j <= 4; j++)
//         {
//             for (i = 1; i <= 128; i++)
//             {
//                 a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave[k - 1][j][i] + ALPHA_ZZP * a0_slave[k + 1][j][i];
//             }
//         }
//     }
//     k = 0;
//     for (j = 1; j <= 4; j++)
//     {
//         for (i = 1; i <= 128; i++)
//         {
//             a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave_lr[0][j - 1][i - 1] + ALPHA_ZZP * a0_slave[k + 1][j][i];
//         }
//     }
//     k = 3;
//     for (j = 1; j <= 4; j++)
//     {
//         for (i = 1; i <= 128; i++)
//         {
//             a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave[k - 1][j][i] + ALPHA_ZZP * a0_slave_lr[1][j - 1][i - 1];
//         }
//     }

//     //put
//     put_reply = 0;
//     //dma_descriptor_init(&put_desc, put_reply);
//     dma_set_reply(&put_desc, &put_reply);
//     dma_set_size(&put_desc, 128 * 8);
//     for (k = z; k < z + 4; k++)
//     {
//         for (j = y; j < y + 4; j++)
//         {
//             dma(put_desc, &a1[INDEX(x, j, k, ldx, ldy)], &a1_slave[k - z][j - y][0]);
//             // athread_put(PE_MODE, &a1_slave[k - z][j - y][0], &a1[INDEX(x, j, k, ldx, ldy)], 128 * 8, &put_reply, 0, 0);
//         }
//     }

//     dma_wait(&put_reply, 16);
//     // while (put_reply != 16)
//     //     ;
//     return;
// }

void funcee()
{
    //512*512*32->128*32*32->128*4*4
    volatile int i, j, k, cpe_id, px, py, pz, t, x, y, z;
    cpe_id = athread_get_id(-1);
    px = 128 * (cpe_id % 4) + 1;
    py = 32 * (cpe_id / 4) + 1;
    pz = 1;
    dma_desc get_desc;
    dma_desc put_desc;
    dma_descriptor_init(&get_desc, 0);
    dma_descriptor_init(&put_desc, 0);
    dma_set_op(&put_desc, DMA_PUT);

    for (t = 0; t < 64; t++)
    {
        x = px;
        y = py + 4 * (t % 8);
        z = pz + 4 * (t / 8);
        //cal_inner(x, y, z);
        //printf("done:%d\n",t);
        //int i, j, k;
        // get_reply = 0;
        // put_reply = 0;
        get_reply = 0;
        dma_descriptor_init(&get_desc, get_reply);
        //get
        dma_set_reply(&get_desc, &get_reply);
        dma_set_size(&get_desc, 130 * 8);
        for (k = z; k < z + 4; k++)
        {
            for (j = y - 1; j <= y + 4; j++)
            {
                dma(get_desc, &a0[INDEX(x - 1, j, k, ldx, ldy)], &a0_slave[k - z][j - y + 1][0]);
                //athread_get(PE_MODE, &a0[INDEX(x - 1, j, k, ldx, ldy)], &a0_slave[k - z][j - y + 1][0], 130 * 8, &get_reply, 0, 0, 0);
            }
        }

        //dma_wait(&get_reply, 24);
        //get_reply=0;
        //dma_set_reply(&get_desc, &get_reply);
        dma_set_size(&get_desc, 128 * 8);
        for (j = y; j < y + 4; j++)
        {
            dma(get_desc, &a0[INDEX(x, j, z - 1, ldx, ldy)], &a0_slave_lr[0][j - y][0]);
            dma(get_desc, &a0[INDEX(x, j, z + 4, ldx, ldy)], &a0_slave_lr[1][j - y][0]);
            // athread_get(PE_MODE, &a0[INDEX(x, j, z - 1, ldx, ldy)], &a0_slave_lr[0][j - y][0], 128 * 8, &get_reply, 0, 0, 0);
            // athread_get(PE_MODE, &a0[INDEX(x, j, z + 4, ldx, ldy)], &a0_slave_lr[1][j - y][0], 128 * 8, &get_reply, 0, 0, 0);
        }

        //printf("get:%d\n", get_reply);
        //dma_wait(&get_reply, 8);
        dma_wait(&get_reply, 32);
        // while (get_reply != 32) //24+4+4
        //     ;
        //printf("get done!");
        //cal
        for (k = 1; k < 3; k++)
        {
            for (j = 1; j <= 4; j++)
            {
                for (i = 1; i <= 128; i++)
                {
                    a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave[k - 1][j][i] + ALPHA_ZZP * a0_slave[k + 1][j][i];
                }
            }
        }
        k = 0;
        for (j = 1; j <= 4; j++)
        {
            for (i = 1; i <= 128; i++)
            {
                a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave_lr[0][j - 1][i - 1] + ALPHA_ZZP * a0_slave[k + 1][j][i];
            }
        }
        k = 3;
        for (j = 1; j <= 4; j++)
        {
            for (i = 1; i <= 128; i++)
            {
                a1_slave[k][j - 1][i - 1] = ALPHA_ZZZ * a0_slave[k][j][i] + ALPHA_NZZ * a0_slave[k][j][i - 1] + ALPHA_PZZ * a0_slave[k][j][i + 1] + ALPHA_ZNZ * a0_slave[k][j - 1][i] + ALPHA_ZPZ * a0_slave[k][j + 1][i] + ALPHA_ZZN * a0_slave[k - 1][j][i] + ALPHA_ZZP * a0_slave_lr[1][j - 1][i - 1];
            }
        }

        //put
        put_reply = 0;
        //dma_descriptor_init(&put_desc, put_reply);
        dma_set_reply(&put_desc, &put_reply);
        dma_set_size(&put_desc, 128 * 8);
        for (k = z; k < z + 4; k++)
        {
            for (j = y; j < y + 4; j++)
            {
                dma(put_desc, &a1[INDEX(x, j, k, ldx, ldy)], &a1_slave[k - z][j - y][0]);
                // athread_put(PE_MODE, &a1_slave[k - z][j - y][0], &a1[INDEX(x, j, k, ldx, ldy)], 128 * 8, &put_reply, 0, 0);
            }
        }

        dma_wait(&put_reply, 16);
    }
}