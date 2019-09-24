CC=g++
CFLAG=-Wno-deprecated
CFLAG_OPT=-std=gnu++0x
PERM=-fpermissive
INC=-I/users/outil/dsx/cctools/include
INC_SCASS=-I/users/outil/dsx/systemcass/include
LD=-L/users/outil/dsx/cctools/lib-linux
LD_SCASS=-L/users/outil/dsx/systemcass/lib
OPT=-m32


all:fast_simulator.x simulator.x

list=fifo_gcd_master.o fifo_gcd_coprocessor.o tp1_top.o
fast_list=fast_fifo_gcd_master.o fast_fifo_gcd_coprocessor.o fast_tp1_top.o


fast_simulator.x: $(fast_list)
	$(CC) $(CFLAG) $(PERM) $(OPT) -L. $(LD_SCASS) -o $@ $^ -lsystemc -ldl 2>&1
fast_fifo_gcd_coprocessor.o : fifo_gcd_coprocessor.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC_SCASS) $(OPT) -c $< -o $@
fast_fifo_gcd_master.o : fifo_gcd_master.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC_SCASS) $(OPT) -c $< -o $@
fast_tp1_top.o : tp1_top.cpp 
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC_SCASS) $(OPT) -c $< -o $@


simulator.x: $(list)
	$(CC) $(CFLAG) $(PERM) $(OPT) -L. $(LD) -o $@ $^ -lsystemc 2>&1
fifo_gcd_coprocessor.o : fifo_gcd_coprocessor.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC) $(OPT) -c $<
fifo_gcd_master.o : fifo_gcd_master.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC) $(OPT) -c $<
tp1_top.o : tp1_top.cpp 
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC) $(OPT) -c $<


clean:
	rm -f *.o *.bin core *~ simulator.x fast_simulator.x
