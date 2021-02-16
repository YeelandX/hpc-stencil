
#CC = mpicc #mpiicc 
LD = mpicc #mpiicc 
CC = sw5cc

OPT =
#CFLAGS = -Wall -std=c99 $(OPT) -OPT:IEEE_arith=1  #-qopenmp
#CFLAGS = -O3 -host -I/usr/sw-mpp/mpi2/include/ -msimd -lm -Wall -OPT:alias=disjoint -std=c99 $(OPT) -OPT:IEEE_arith=1
CFLAGS = -O3 -host -I/usr/sw-mpp/mpi2/include/ -msimd -OPT:alias=disjoint -lm -Wall $(OPT) -OPT:IEEE_arith=1 -std=gnu9x -OPT:Ofast -OPT:roundoff=3#rpcc
#slave
#SFLAGS = -O2 -slave -lm_slave
SFLAGS = -O3 -slave  -lm_slave -msimd -OPT:alias=disjoint -OPT:Ofast -OPT:roundoff=3
#SFLAGS = -O3 -slave  -lm_slave -msimd -OPT:IEEE_arithmetic=3 -OPT:roundoff=3
LDFLAGS = -Wall #-qopenmp
LDLIBS = $(LDFLAGS)

#targets = benchmark-naive benchmark-optimized
targets = benchmark-optimized
#objects = check.o benchmark.o stencil-naive.o stencil-optimized.o slave_7.o slave_27.o
objects = check.o benchmark.o stencil-optimized.o slave_7.o slave_27.o

.PHONY : default
default : all

.PHONY : all
all : clean $(targets)

#benchmark-naive : check.o benchmark.o stencil-naive.o
#	$(LD) -o $@ $^ $(LDLIBS)

benchmark-optimized : check.o benchmark.o stencil-optimized.o slave_7.o
	$(LD) -o $@ $^ $(LDLIBS)

#add
stencil-optimized.o:stencil-optimized.c
	$(CC) $(CFLAGS) -c $<
slave_27.o:slave_27.c
	$(CC) $(SFLAGS) -c $<
slave_7.o:slave_7.c
	$(CC) $(SFLAGS) -c $<
check.o:check.c
	$(CC) $(CFLAGS) -c $<
benchmark.o:benchmark.c
	$(CC) $(CFLAGS) -c $<

#%.o : %.c common.h
#	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(targets) $(objects)
