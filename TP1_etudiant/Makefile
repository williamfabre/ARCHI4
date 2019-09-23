CC=g++
CFLAG=-Wno-deprecated -fpermissive -std=gnu++0x
INC=-I/users/outil/dsx/cctools/include
LD=-L/users/outil/dsx/cctools/lib-linux
OPT=-m32


all:simulator.x

list=fifo_gcd_master.o fifo_gcd_coprocessor.o tp1_top.o


simulator.x: $(list)
	$(CC) $(CFLAG) -L. $(LD) -o $@ $< -lsystemc 2>&1

fifo_gcd_coprocessor.o : fifo_gcd_coprocessor.cpp
	$(CC) $(CFLAG) -I. $(INC) $(OPT) -c $<
fifo_gcd_master.o : fifo_gcd_master.cpp
	$(CC) $(CFLAG) -I. $(INC) $(OPT) -c $<
tp1_top.o : tp1_top.cpp 
	$(CC) $(CFLAG) -I. $(INC) $(OPT) -c $<

clean:
	rm -f *.o *.bin core *~ simulator.x
