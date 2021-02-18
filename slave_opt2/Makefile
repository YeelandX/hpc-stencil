CC = mpicc
SLAVECC = sw5cc -slave
OPT =
CFLAGS = -Wall -std=c99 -g -O3 $(OPT) -OPT:IEEE_arith=1 -msimd
CSLAVEFLAGS = -Wall -g -O3 $(OPT) -OPT:IEEE_arith=1 -msimd

LDFLAGS = -Wall
LDLIBS = $(LDFLAGS)

targets = benchmark-naive benchmark-optimized
objects = check.o benchmark.o mpi_comm_helper.o stencil-naive.o stencil-optimized.o 

slave_objects = athread_7_y_ldm_reg.o athread_7_y_ldm_prefetch_reg.o  athread_halo_7_z_ldm_reg.o \
                athread_27_y_ldm_reg_simd.o athread_27_y_ldm_prefetch_reg_simd.o athread_halo_27_z_ldm_reg_simd.o                                         
host_common_objects = host_common.o
slave_common_objects = slave_comm.o comp_utils.o


.PHONY : default
default : all

.PHONY : all
all : clean $(targets)

benchmark-naive : check.o benchmark.o stencil-naive.o
	$(CC) -o $@ $^ $(LDLIBS)

benchmark-optimized : check.o benchmark.o mpi_comm_helper.o host_common.o slave_comm.o comp_utils.o           \
                      stencil-optimized.o athread_7_y_ldm_reg.o athread_halo_7_z_ldm_reg.o  athread_7_y_ldm_prefetch_reg.o \
					  athread_27_y_ldm_reg_simd.o athread_27_y_ldm_prefetch_reg_simd.o athread_halo_27_z_ldm_reg_simd.o
	$(CC) -o $@ $^ $(LDLIBS)

$(objects): %.o : %.c common.h
	$(CC) -c $(CFLAGS) $< -o $@

$(slave_objects): %.o : %.c common.h
	$(SLAVECC) -c $(CSLAVEFLAGS) $< -o $@

$(host_common_objects): %.o : %.c common.h
	$(CC) -c -Wall -g $(OPT) -OPT:IEEE_arith=1 $< -o $@

$(slave_common_objects): %.o : %.c common.h
	$(SLAVECC) -c $(CSLAVEFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf benchmark-optimized
	rm -rf $(targets)
	rm -rf *.o