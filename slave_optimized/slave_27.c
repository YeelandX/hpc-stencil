#include<stdio.h>
#include<math.h>
#include<string.h>
#include<slave.h>
#include<dma.h>
#include"common.h"

//mylength1为一次DMA的长度
__thread_local int my_id;
__thread_local volatile unsigned long get_reply[2], put_reply[2];
__thread_local double a0_slave_0[2][mylength1 + 2], a0_slave_3[2][mylength1+2], a0_slave_4[2][mylength1+2], a0_slave_5[2][mylength1+2], a0_slave_6[2][mylength1+2], \
a0_slave_15[2][mylength1 + 2], a0_slave_16[2][mylength1 + 2], a0_slave_17[2][mylength1 + 2], a0_slave_18[2][mylength1 + 2], a1_slave_0[2][mylength1];

extern cptr_t a0;
extern ptr_t a1;
extern int ldx, ldy, ldz;

void func_27() {
	int i, t, time;
	int quotient, reminder, my_start, taskcount, my_count, my_reminder;
	int index, last, next;//double buffer
	my_id = athread_get_id(-1);
	
	int xy_dim = ldx * ldy;
	//add msaysn6.9
	if (ldz == 34) {
		taskcount = xy_dim * (ldz - 4);
		quotient = taskcount / 64;
		reminder = taskcount % 64;
		if (my_id < reminder) {
			my_count = quotient + 1;
			my_start = my_id * my_count + 2 * xy_dim;
		}
		else {
			my_count = quotient;
			my_start = my_id * my_count + 2 * xy_dim + reminder;
		}
		time = my_count / mylength1;//每个从核需要time次DMA
	}
	else if (ldz == 1) {
		taskcount = xy_dim;
		quotient = taskcount / 64;
		reminder = taskcount % 64;
		if (my_id < reminder) {
			my_count = quotient + 1;
			my_start = my_id * my_count + xy_dim;
		}
		else {
			my_count = quotient;
			my_start = my_id * my_count + xy_dim + reminder;
		}
		time = my_count / mylength1;//每个从核需要time次DMA
	}
	else if (ldz == 32) {
		taskcount = xy_dim;
		quotient = taskcount / 64;
		reminder = taskcount % 64;
		if (my_id < reminder) {
			my_count = quotient + 1;
			my_start = my_id * my_count + 32 * xy_dim;
		}
		else {
			my_count = quotient;
			my_start = my_id * my_count + 32 * xy_dim + reminder;
		}
		time = my_count / mylength1;//每个从核需要time次DMA
	}
	
	my_reminder = my_count % mylength1;//每个从核剩余的任务数
	int ii, ii_next, own_time;

	dma_desc get_desc;
	dma_descriptor_init(&get_desc, 0);
	dma_desc put_desc;
	dma_descriptor_init(&put_desc, 0);
	dma_set_op(&put_desc, DMA_PUT);

	for (t = 0; t < time; t++) {
		index = t % 2;
		last = (t - 1) % 2;
		next = (t + 1) % 2;
		ii = my_start + t * mylength1;
		ii_next = my_start + (t + 1)*mylength1;
		//dma_descriptor_init(&get_desc, get_reply[index]);
		//dma_set_size(&get_desc, mylength1 * 8);
		if (t == 0) {
			get_reply[index] = 0;
			dma_set_reply(&get_desc, &get_reply[index]);
			dma_set_size(&get_desc, (mylength1 + 2) * 8);
			dma(get_desc, &a0[ii - 1], &a0_slave_0[index][0]);
			//dma_set_size(&get_desc, mylength * 8);
			dma(get_desc, &a0[ii - 1 - ldx], &a0_slave_3[index][0]);
			dma(get_desc, &a0[ii - 1 + ldx], &a0_slave_4[index][0]);
			dma(get_desc, &a0[ii - 1 - xy_dim], &a0_slave_5[index][0]);
			dma(get_desc, &a0[ii - 1 + xy_dim], &a0_slave_6[index][0]);
			//add
			dma(get_desc, &a0[ii - 1 - ldx - xy_dim], &a0_slave_15[index][0]);
			dma(get_desc, &a0[ii - 1 + ldx - xy_dim], &a0_slave_16[index][0]);
			dma(get_desc, &a0[ii - 1 - ldx + xy_dim], &a0_slave_17[index][0]);
			dma(get_desc, &a0[ii - 1 + ldx + xy_dim], &a0_slave_18[index][0]);
		}
		if (t < time - 1) {
			get_reply[next] = 0;
			dma_set_reply(&get_desc, &get_reply[next]);
			dma_set_size(&get_desc, (mylength1 + 2) * 8);
			dma(get_desc, &a0[ii_next - 1], &a0_slave_0[next][0]);
			//dma_set_size(&get_desc, mylength * 8);
			dma(get_desc, &a0[ii_next - 1 - ldx], &a0_slave_3[next][0]);
			dma(get_desc, &a0[ii_next - 1 + ldx], &a0_slave_4[next][0]);
			dma(get_desc, &a0[ii_next - 1 - xy_dim], &a0_slave_5[next][0]);
			dma(get_desc, &a0[ii_next - 1 + xy_dim], &a0_slave_6[next][0]);
			dma(get_desc, &a0[ii_next - 1 - ldx - xy_dim], &a0_slave_15[next][0]);
			dma(get_desc, &a0[ii_next - 1 + ldx - xy_dim], &a0_slave_16[next][0]);
			dma(get_desc, &a0[ii_next - 1 - ldx + xy_dim], &a0_slave_17[next][0]);
			dma(get_desc, &a0[ii_next - 1 + ldx + xy_dim], &a0_slave_18[next][0]);
		}
		dma_wait(&get_reply[index], 9);
		for (i = 0; i < mylength1; i++) {
			//a1_slave_0[index][i] = ALPHA_ZZZ * a0_slave_0[index][i + 1] + ALPHA_NZZ * a0_slave_0[index][i] + ALPHA_PZZ * a0_slave_0[index][i + 2] + ALPHA_ZNZ * a0_slave_3[index][i] + ALPHA_ZPZ * a0_slave_4[index][i] + ALPHA_ZZN * a0_slave_5[index][i] + ALPHA_ZZP * a0_slave_6[index][i];
			a1_slave_0[index][i] = ALPHA_ZZZ * a0_slave_0[index][i + 1] + ALPHA_NZZ * a0_slave_0[index][i] + \
				ALPHA_PZZ * a0_slave_0[index][i + 2] + ALPHA_ZNZ * a0_slave_3[index][i + 1] +\
				ALPHA_ZPZ * a0_slave_4[index][i + 1] + ALPHA_ZZN * a0_slave_5[index][i + 1] +\
				ALPHA_ZZP * a0_slave_6[index][i + 1] + ALPHA_NNZ * a0_slave_3[index][i] +\
				ALPHA_PNZ * a0_slave_3[index][i + 2] + ALPHA_NPZ * a0_slave_4[index][i] +\
				ALPHA_PPZ * a0_slave_4[index][i + 2] + ALPHA_NZN * a0_slave_5[index][i] +\
				ALPHA_PZN * a0_slave_5[index][i + 2] + ALPHA_NZP * a0_slave_6[index][i] +\
				ALPHA_PZP * a0_slave_6[index][i + 2] + ALPHA_ZNN * a0_slave_15[index][i + 1] +\
				ALPHA_ZPN * a0_slave_16[index][i + 1] + ALPHA_ZNP * a0_slave_17[index][i + 1] +\
				ALPHA_ZPP * a0_slave_18[index][i + 1] + ALPHA_NNN * a0_slave_15[index][i] +\
				ALPHA_PNN * a0_slave_15[index][i + 2] + ALPHA_NPN * a0_slave_16[index][i] +\
				ALPHA_PPN * a0_slave_16[index][i + 2] + ALPHA_NNP * a0_slave_17[index][i] +\
				ALPHA_PNP * a0_slave_17[index][i + 2] + ALPHA_NPP * a0_slave_18[index][i] +\
				ALPHA_PPP * a0_slave_18[index][i + 2];
		}//计算
		put_reply[index] = 0;
		dma_set_size(&put_desc, mylength1 * 8);
		dma_set_reply(&put_desc, &put_reply[index]);
		dma(put_desc, &a1[ii], &a1_slave_0[index][0]);
		if (t > 0) {
			dma_wait(&put_reply[last], 1);
		}
		if (t == time - 1) {
			dma_wait(&put_reply[index], 1);
		}
	}
	
	own_time = my_start + time * mylength1;
	
	get_reply[0] = 0;
	dma_set_reply(&get_desc, &get_reply[0]);
	dma_set_size(&get_desc, (my_reminder + 2) * 8);
	dma(get_desc, &a0[own_time - 1], &a0_slave_0[0][0]);
	//dma_set_size(&get_desc, my_reminder * 8);
	dma(get_desc, &a0[own_time - 1 - ldx], &a0_slave_3[0][0]);
	dma(get_desc, &a0[own_time - 1 + ldx], &a0_slave_4[0][0]);
	dma(get_desc, &a0[own_time - 1 - xy_dim], &a0_slave_5[0][0]);
	dma(get_desc, &a0[own_time - 1 + xy_dim], &a0_slave_6[0][0]);
	//add
	dma(get_desc, &a0[own_time - 1 - ldx - xy_dim], &a0_slave_15[0][0]);
	dma(get_desc, &a0[own_time - 1 + ldx - xy_dim], &a0_slave_16[0][0]);
	dma(get_desc, &a0[own_time - 1 - ldx + xy_dim], &a0_slave_17[0][0]);
	dma(get_desc, &a0[own_time - 1 + ldx + xy_dim], &a0_slave_18[0][0]);

	dma_wait(&get_reply[0], 9);
	for (i = 0; i < my_reminder; i++) {
		//a1_slave_0[0][i] = ALPHA_ZZZ * a0_slave_0[0][i + 1] + ALPHA_NZZ * a0_slave_0[0][i] + ALPHA_PZZ * a0_slave_0[0][i + 2] + ALPHA_ZNZ * a0_slave_3[0][i] + ALPHA_ZPZ * a0_slave_4[0][i] + ALPHA_ZZN * a0_slave_5[0][i] + ALPHA_ZZP * a0_slave_6[0][i];
		a1_slave_0[0][i] = ALPHA_ZZZ * a0_slave_0[0][i + 1] + ALPHA_NZZ * a0_slave_0[0][i] +\
			ALPHA_PZZ * a0_slave_0[0][i + 2] + ALPHA_ZNZ * a0_slave_3[0][i + 1] +\
			ALPHA_ZPZ * a0_slave_4[0][i + 1] + ALPHA_ZZN * a0_slave_5[0][i + 1] +\
			ALPHA_ZZP * a0_slave_6[0][i + 1] + ALPHA_NNZ * a0_slave_3[0][i] +\
			ALPHA_PNZ * a0_slave_3[0][i + 2] + ALPHA_NPZ * a0_slave_4[0][i] +\
			ALPHA_PPZ * a0_slave_4[0][i + 2] + ALPHA_NZN * a0_slave_5[0][i] +\
			ALPHA_PZN * a0_slave_5[0][i + 2] + ALPHA_NZP * a0_slave_6[0][i] +\
			ALPHA_PZP * a0_slave_6[0][i + 2] + ALPHA_ZNN * a0_slave_15[0][i + 1] +\
			ALPHA_ZPN * a0_slave_16[0][i + 1] + ALPHA_ZNP * a0_slave_17[0][i + 1] +\
			ALPHA_ZPP * a0_slave_18[0][i + 1] + ALPHA_NNN * a0_slave_15[0][i] +\
			ALPHA_PNN * a0_slave_15[0][i + 2] + ALPHA_NPN * a0_slave_16[0][i] +\
			ALPHA_PPN * a0_slave_16[0][i + 2] + ALPHA_NNP * a0_slave_17[0][i] +\
			ALPHA_PNP * a0_slave_17[0][i + 2] + ALPHA_NPP * a0_slave_18[0][i] +\
			ALPHA_PPP * a0_slave_18[0][i + 2];
	}
	put_reply[0] = 0;
	dma_set_size(&put_desc, my_reminder * 8);
	dma_set_reply(&put_desc, &put_reply[0]);
	dma(put_desc, &a1[own_time], &a1_slave_0[0][0]);
	dma_wait(&put_reply[0], 1);
}
