#ifndef COMM_H_INCLUDE
#define COMM_H_INCLUDE
#include <stdio.h>

#define REG_PUTR(var, dst) asm volatile("putr %0,%1\n" ::"r"(var), "r"(dst))
#define REG_PUTC(var, dst) asm volatile("putc %0,%1\n" ::"r"(var), "r"(dst))
#define REG_GETR(var) asm volatile("getr %0\n":"=r"(var))
#define REG_GETC(var) asm volatile("getc %0\n":"=r"(var))

// s comm toolkit
inline int s_id(const int id);
inline int s_next_id(const int id);
inline int s_prev_id(const int id);
inline void s_sync(const int id,const int another);
void send_to_d(const int id, const int to, double *data, const int len);
void recv_from_d(const int id, const int from, double *data, const int len);



#define A_DMA_IGET(mode, src_p_p, ldm_p, len, reply_p) \
    ({                                                 \
        dma_desc __da__ = 0;                           \
        dma_set_op(&__da__, DMA_GET);                  \
        dma_set_mode(&__da__, mode);                   \
        dma_set_size(&__da__, len);                    \
        dma_set_reply(&__da__, reply_p);               \
        dma(__da__, src_p_p, ldm_p);                   \
    })

/*backup athread_get*/
#define A_DMA_GET(mode, src_p_p, ldm_p, len, reply_p) \
    ({                                                \
        dma_desc __da__;                              \
        dma_set_op(&__da__, DMA_GET);                 \
        dma_set_mode(&__da__, mode);                  \
        dma_set_size(&__da__, len);                   \
        dma_set_reply(&__da__, reply_p);              \
        dma(__da__, src_p_p, ldm_p);                  \
        dma_wait(reply_p, 1);                         \
    })

#define A_DMA_GET_SET(da, mode, len, reply_p) \
    ({                                        \
        dma_set_op(&da, DMA_GET);             \
        dma_set_mode(&da, mode);              \
        dma_set_size(&da, len);               \
        dma_set_reply(&da, reply_p);          \
    })

#define A_DMA_STEP_GET_SET(da, mode, len, reply_p, stride, bsize) \
    ({                                                            \
        dma_set_op(&da, DMA_GET);                                 \
        dma_set_mode(&da, mode);                                  \
        dma_set_size(&da, len);                                   \
        dma_set_reply(&da, reply_p);                              \
        dma_set_stepsize(&da, stride);                            \
        dma_set_bsize(&da, bsize);                                \
    })

#define A_DMA_PUT_SET(da, mode, len, reply_p) \
    ({                                        \
        dma_set_op(&da, DMA_PUT);             \
        dma_set_mode(&da, mode);              \
        dma_set_size(&da, len);               \
        dma_set_reply(&da, reply_p);          \
    })

#define A_DMA_STEP_PUT_SET(da, mode, len, reply_p, stride, bsize) \
    ({                                                            \
        dma_set_op(&da, DMA_PUT);                                 \
        dma_set_mode(&da, mode);                                  \
        dma_set_size(&da, len);                                   \
        dma_set_reply(&da, reply_p);                              \
        dma_set_stepsize(&da, stride);                            \
        dma_set_bsize(&da, bsize);                                \
    })

#define A_DMA_RUN(da, mem_p, ldm_p) ({ \
    dma(da, mem_p, ldm_p);             \
})

#define A_DMA_WAIT(reply_p, n) ({ \
    dma_wait((reply_p), n);       \
})

#endif // !COMM_H_INCLUDE
