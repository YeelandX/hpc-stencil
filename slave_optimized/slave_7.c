#include<stdio.h>
#include<math.h>
#include<string.h>
#include<slave.h>
#include<dma.h>
#include"common.h"
#define mylength 130 //128+2
//#define mylength 258 //256+2
#define taskcount 8388608 //512*512*32
#define ldx 514
#define ldy 514
#define ldz 34
#define xy_dim 264196

//mylength为一次DMA的长度
__thread_local int cpe_id;
__thread_local volatile unsigned long get_reply, put_reply;
/*__thread_local double a0_slave_1[mylength], a0_slave_2[mylength],a0_slave_4[mylength], a0_slave_5[mylength], \
a0_slave_6[mylength], a0_slave_7[mylength], a0_slave_8[mylength], a0_slave_9[mylength], a0_slave_10[mylength],\
a0_slave_11[mylength],  a0_slave_13[mylength], a0_slave_14[mylength], a1_slave_5[mylength], a1_slave_6[mylength], \
a1_slave_9[mylength], a1_slave_10[mylength];tpye2*2*/
__thread_local double a0_slave_1[4][mylength], a0_slave_10[6][mylength], a0_slave_20[6][mylength], \
a0_slave_30[6][mylength], a0_slave_40[6][mylength], a0_slave_51[4][mylength],
a1_slave_10[4][mylength], a1_slave_20[4][mylength], a1_slave_30[4][mylength], a1_slave_40[4][mylength];
extern cptr_t a0;
extern ptr_t a1;

void func_7() {
	int i, j, iter, t, ii, xx, yy, zz;//xx,yy,zz为主存中的地址
	//int cldx, cldy, cldz;
	//cldx = 130; cldy = 6; cldz = 6;
	cpe_id = athread_get_id(-1);
	//iter = taskcount / (64 * 128 * 2 * 2);//块状大小为128*2*2
	//iter = taskcount / (64 * 256 * 2 * 2);
	iter = taskcount / (64 * 128 * 4 * 4);
	//add dma
	dma_desc get_desc;
	dma_descriptor_init(&get_desc, 0);
	dma_desc put_desc;
	dma_descriptor_init(&put_desc, 0);
	dma_set_op(&put_desc, DMA_PUT);

	for (t = 0; t < iter; t++) {
		//512*512*32->128*32*32->128*2*2
		/*xx = (cpe_id % 4) * 128 + 1;
		yy = (cpe_id / 4 * 32)+(t % 32) * 2 % 32 + 1;
		zz = 2 * (t / 16) + 1;*/
		//512*512*32->256*16*32->256*2*2
		/*xx = (cpe_id % 2) * 256 + 1;
		yy = (cpe_id / 2 * 16) + (t % 16) * 2 % 16 + 1;
		zz = 2 * (t / 8) + 1;*/
		//512*512*32->128*32*32->64*8*8
		/*xx = (cpe_id % 4 * 128) + (t % 2 * 64) + 1;
		yy = (cpe_id / 4 * 32) + (t / 2) * 8 % 32 + 1;
		zz = 8 * (t / 8) + 1;*/
		//512*512*32->128*32*32->32*8*8
		/*xx = (cpe_id % 4 * 128) + (t % 4 * 32) + 1;
		yy = (cpe_id / 4 * 32) + (t / 4) * 8 % 32 + 1;
		zz = 8 * (t / 16) + 1;*/
		//512 * 512 * 32->128 * 32 * 32->128 * 4 * 4
		xx = (cpe_id % 4 * 128) + 1;
		yy = (cpe_id / 4 * 32) + (t % 8) * 4 + 1;
		zz = 4 * (t / 8) + 1;
		ii = INDEX(xx, yy, zz, ldx, ldy);
		dma_descriptor_init(&get_desc, get_reply);
		get_reply = 0;
		//continus
		/*dma_set_reply(&get_desc, &get_reply);
		dma_set_size(&get_desc, mylength * 8);
		dma(get_desc, &a0[ii - xy_dim - 1], &a0_slave_1[0]);
		dma(get_desc, &a0[ii - xy_dim +ldx- 1], &a0_slave_2[0]);
		dma(get_desc, &a0[ii - ldx - 1], &a0_slave_4[0]);
		dma(get_desc, &a0[ii  - 1], &a0_slave_5[0]);
		dma(get_desc, &a0[ii +ldx - 1], &a0_slave_6[0]);
		dma(get_desc, &a0[ii +2*ldx - 1], &a0_slave_7[0]);
		dma(get_desc, &a0[ii + xy_dim-ldx - 1], &a0_slave_8[0]);
		dma(get_desc, &a0[ii + xy_dim - 1], &a0_slave_9[0]);
		dma(get_desc, &a0[ii + xy_dim + ldx - 1], &a0_slave_10[0]);
		dma(get_desc, &a0[ii + xy_dim + 2 * ldx - 1], &a0_slave_11[0]);
		dma(get_desc, &a0[ii + 2 * xy_dim - 1], &a0_slave_13[0]);
		dma(get_desc, &a0[ii + 2 * xy_dim + ldx - 1], &a0_slave_14[0]);
		dma_wait(&get_reply, 12);*/
		//stride
		/*dma_set_reply(&get_desc, &get_reply);
		dma_set_size(&get_desc, cldx * 8 * 8);
		dma_set_bsize(&get_desc, cldx * 8);
		dma_set_stepsize(&get_desc, (ldx - cldx) * 8);

		dma(get_desc, &a0[ii - xy_dim - 1], &a0_slave_1[0]);
		dma(get_desc, &a0[ii + 8 * xy_dim - 1], &a0_slave_91[0]);

		dma_set_size(&get_desc, cldx * 10 * 8);
		dma(get_desc, &a0[ii - ldx - 1], &a0_slave_10[0]);
		dma(get_desc, &a0[ii + xy_dim - ldx - 1], &a0_slave_20[0]);
		dma(get_desc, &a0[ii + 2 * xy_dim - ldx - 1], &a0_slave_30[0]);
		dma(get_desc, &a0[ii + 3 * xy_dim - ldx - 1], &a0_slave_40[0]);
		dma(get_desc, &a0[ii + 4 * xy_dim - ldx - 1], &a0_slave_50[0]);
		dma(get_desc, &a0[ii + 5 * xy_dim - ldx - 1], &a0_slave_60[0]);
		dma(get_desc, &a0[ii + 6 * xy_dim - ldx - 1], &a0_slave_70[0]);
		dma(get_desc, &a0[ii + 7 * xy_dim - ldx - 1], &a0_slave_80[0]);
		dma_wait(&get_reply, 10);*/
		//
		dma_set_reply(&get_desc, &get_reply);
		dma_set_size(&get_desc, mylength * 8);
		for (i = 0; i < 4; i++) {
			//get_reply = 0;
			dma(get_desc, &a0[ii - xy_dim - 1 + i * ldx], &a0_slave_1[i][0]);
			dma(get_desc, &a0[ii + 4 * xy_dim - 1 + i * ldx], &a0_slave_51[i][0]);
			//dma_wait(&get_reply, 2);
		}
		for (i = 0; i < 6; i++) {
			//get_reply = 0;
			dma(get_desc, &a0[ii - ldx - 1 + i * ldx], &a0_slave_10[i][0]);
			dma(get_desc, &a0[ii + xy_dim - ldx - 1 + i * ldx], &a0_slave_20[i][0]);
			dma(get_desc, &a0[ii + 2 * xy_dim - ldx - 1 + i * ldx], &a0_slave_30[i][0]);
			dma(get_desc, &a0[ii + 3 * xy_dim - ldx - 1 + i * ldx], &a0_slave_40[i][0]);
			
			//dma_wait(&get_reply, 8);
		}
		dma_wait(&get_reply, 32);
		
		/*athread_get(PE_MODE, &a0[ii - xy_dim - 1], &a0_slave_1[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii - xy_dim + ldx - 1], &a0_slave_2[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii - ldx - 1], &a0_slave_4[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii - 1], &a0_slave_5[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + ldx - 1], &a0_slave_6[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + 2 * ldx - 1], &a0_slave_7[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + xy_dim - ldx - 1], &a0_slave_8[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + xy_dim - 1], &a0_slave_9[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + xy_dim + ldx - 1], &a0_slave_10[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + xy_dim + 2 * ldx - 1], &a0_slave_11[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + 2 * xy_dim - 1], &a0_slave_13[0], mylength * 8, &get_reply, 0, 0, 0);
		athread_get(PE_MODE, &a0[ii + 2 * xy_dim + ldx - 1], &a0_slave_14[0], mylength * 8, &get_reply, 0, 0, 0);
		while (get_reply != 12);*/
		
		/*1*/
		for (j = 0; j < 4; j++){
			for (i = 1; i <= mylength - 2; i++) {
				a1_slave_10[j][i] = ALPHA_ZZZ * a0_slave_10[j + 1][i] + ALPHA_NZZ * a0_slave_10[j + 1][i - 1] + ALPHA_PZZ * a0_slave_10[j + 1][i + 1] \
					+ ALPHA_ZNZ * a0_slave_10[j][i] + ALPHA_ZPZ * a0_slave_10[j + 2][i] + ALPHA_ZZN * a0_slave_1[j][i] \
					+ ALPHA_ZZP * a0_slave_20[j + 1][i];
			}
		}
		/*2*/
		for (j = 0; j < 4; j++) {
			for (i = 1; i <= mylength - 2; i++) {
				a1_slave_20[j][i] = ALPHA_ZZZ * a0_slave_20[j + 1][i] + ALPHA_NZZ * a0_slave_20[j + 1][i - 1] + ALPHA_PZZ * a0_slave_20[j + 1][i + 1] \
					+ ALPHA_ZNZ * a0_slave_20[j][i] + ALPHA_ZPZ * a0_slave_20[j + 2][i] + ALPHA_ZZN * a0_slave_10[j + 1][i] \
					+ ALPHA_ZZP * a0_slave_30[j + 1][i];
			}
		}
		/*3*/
		for (j = 0; j < 4; j++) {
			for (i = 1; i <= mylength - 2; i++) {
				a1_slave_30[j][i] = ALPHA_ZZZ * a0_slave_30[j + 1][i] + ALPHA_NZZ * a0_slave_30[j + 1][i - 1] + ALPHA_PZZ * a0_slave_30[j + 1][i + 1] \
					+ ALPHA_ZNZ * a0_slave_30[j][i] + ALPHA_ZPZ * a0_slave_30[j + 2][i] + ALPHA_ZZN * a0_slave_20[j + 1][i] \
					+ ALPHA_ZZP * a0_slave_40[j + 1][i];
			}
		}
		/*4*/
		for (j = 0; j < 4; j++) {
			for (i = 1; i <= mylength - 2; i++) {
				a1_slave_40[j][i] = ALPHA_ZZZ * a0_slave_40[j + 1][i] + ALPHA_NZZ * a0_slave_40[j + 1][i - 1] + ALPHA_PZZ * a0_slave_40[j + 1][i + 1] \
					+ ALPHA_ZNZ * a0_slave_40[j][i] + ALPHA_ZPZ * a0_slave_40[j + 2][i] + ALPHA_ZZN * a0_slave_30[j + 1][i] \
					+ ALPHA_ZZP * a0_slave_51[j][i];
			}
		}
		put_reply = 0;
		/*athread_put(PE_MODE, &a1_slave_5[1], &a1[ii], (mylength-2) * 8, &put_reply, 0, 0);
		athread_put(PE_MODE, &a1_slave_6[1], &a1[ii + ldx], (mylength-2) * 8, &put_reply, 0, 0);
		athread_put(PE_MODE, &a1_slave_9[1], &a1[ii + xy_dim], (mylength-2) * 8, &put_reply, 0, 0);
		athread_put(PE_MODE, &a1_slave_10[1], &a1[ii + xy_dim + ldx], (mylength-2) * 8, &put_reply, 0, 0);
		while (put_reply != 4);	*/
		/*dma_set_size(&put_desc, (mylength - 2) * 8);
		dma_set_reply(&put_desc, &put_reply);
		dma_set_size(&put_desc, (cldx - 2) * 8 * 8);
		dma_set_bsize(&put_desc, (cldx - 2) * 8);
		dma_set_stepsize(&put_desc, (ldx - (cldx - 2)) * 8);
		dma(put_desc, &a1[ii], &a1_slave_10[1]);
		dma(put_desc, &a1[ii + xy_dim], &a1_slave_20[1]);
		dma(put_desc, &a1[ii + 2 * xy_dim], &a1_slave_30[1]);
		dma(put_desc, &a1[ii + 3 * xy_dim], &a1_slave_40[1]);
		dma(put_desc, &a1[ii + 4 * xy_dim], &a1_slave_50[1]);
		dma(put_desc, &a1[ii + 5 * xy_dim], &a1_slave_60[1]);
		dma(put_desc, &a1[ii + 6 * xy_dim], &a1_slave_70[1]);
		dma(put_desc, &a1[ii + 7 * xy_dim], &a1_slave_80[1]);
		dma_wait(&put_reply, 8);*/
		//
		dma_set_reply(&put_desc, &put_reply);
		dma_set_size(&put_desc, (mylength - 2) * 8);
		for (i = 0; i < 4; i++) {
			dma(put_desc, &a1[ii + i * ldx], &a1_slave_10[i][1]);
			dma(put_desc, &a1[ii + xy_dim + i * ldx], &a1_slave_20[i][1]);
			dma(put_desc, &a1[ii + 2 * xy_dim + i * ldx], &a1_slave_30[i][1]);
			dma(put_desc, &a1[ii + 3 * xy_dim  + i * ldx], &a1_slave_40[i][1]);
		}
		dma_wait(&put_reply, 16);
	}
}
