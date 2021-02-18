#include "slave_comm.h"
#include <simd.h>
#include <slave.h>


inline int s_id(int id)
{
    int row_id = id >> 3;
    int col_id = id & 7;
    return row_id & 1 ? row_id * 8 + 7 - col_id : row_id * 8 + col_id;
}

inline int s_next_id(int id)
{
    // assert id < 63
    if (id == 56)
        return -1;
    int row_id = id >> 3;//id/2/2/2 i/8
    int col_id = id & 7;//0~7 i%7
    return row_id & 1 ? (col_id == 0 ? id + 8 : id - 1) : (col_id == 7 ? id + 8 : id + 1);
}

inline int s_prev_id(int id)
{
    // assert id > 0
    if (id == 0)
        return -1;
    int row_id = id >> 3;
    int col_id = id & 7;
    return row_id & 1 ? (col_id == 7 ? id - 8 : id + 1) : (col_id == 0 ? id - 8 : id - 1);
}

inline void s_sync(const int id, const int another_id)
{
    if (id == -1 || another_id == -1 || id == another_id)
    {
        return;
    }
    int row_id = id >> 3;
    int col_id = id & 7;
    int row_another_id = another_id >> 3;
    int col_another_id = another_id & 7;
    if (row_id == row_another_id)
    {
        athread_syn(ROW_SCOPE, (1<<col_id)|(1<<col_another_id));
    }
    else if (col_id == col_another_id)
    {
        athread_syn(COL_SCOPE, (1<<row_id)|(1<<row_another_id));
    }else{
        // assert not arrive here
        printf("error in s_sync between %d and %d", id, another_id);
        exit(-1);
    }
}

void send_to_d(const int id, const int to, double *data, const int len)
{
    // assert id > 0
    if (to == -1)
        return;
    int row_id = id >> 3;
    int col_id = id & 7;
    int to_row_id = to >> 3;
    int to_col_id = to & 7;
    doublev4 v;
    int i, j;
    int vlen = len - 4;
    if (row_id == to_row_id)
    {
        for (i = 0; i <= vlen; i += 4)
        {
            simd_load(v, &data[i]);
            REG_PUTR(v, to_col_id);
        }
        for (j = 0; i < len; ++i, ++j)
        {
            ((double *)(&v))[j] = data[i];
        }
        if (j > 0)
            REG_PUTR(v, to_col_id);
    }
    else if (col_id == to_col_id)
    {
        for (i = 0; i <= vlen; i += 4)
        {
            simd_load(v, &data[i]);
            REG_PUTC(v, to_row_id);
        }
        for (j = 0; i < len; ++i)
        {
            ((double *)(&v))[j++] = data[i];
        }
        if (j > 0)
            REG_PUTC(v, to_row_id);
    }
    else
    {
        // assert not arrive here
        printf("error in sent to id %d to %d", id, to);
        exit(-1);
    }
}

void recv_from_d(const int id, const int from, double *data, const int len)
{
    if (from == -1)
        return;
    // assert id > 0
    int row_id = id >> 3;
    int col_id = id & 7;
    int from_row_id = from >> 3;
    int from_col_id = from & 7;
    doublev4 v;
    int i, j;
    int vlen = len - 4;
    if (row_id == from_row_id)
    {
        for (i = 0; i <= vlen; i += 4)
        {
            REG_GETR(v);
            simd_store(v, &data[i]);
        }
        if (i != len)
        {
            REG_GETR(v);
            for (j = 0; i < len; ++i, ++j)
            {
                data[i] = ((double *)(&v))[j];
            }
        }
    }
    else if (col_id == from_col_id)
    {
        for (i = 0; i <= vlen; i += 4)
        {
            REG_GETC(v);
            simd_store(v, &data[i]);
        }
        if (i != len)
        {
            REG_GETC(v);
            for (j = 0; i < len; ++i, ++j)
            {
                data[i] = ((double *)(&v))[j];
            }
        }
    }
    else
    {
        // assert not arrive here
        printf("error in recv from id %d to %d", from, id);
        exit(-1);
    }
}
