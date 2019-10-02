CC=g++
CFLAG=-Wno-deprecated
CFLAG_OPT=-std=gnu++0x
PERM=-fpermissive
INC=-I/users/outil/dsx/cctools/include
INC_SCASS=-I/users/outil/dsx/systemcass/include
LD=-L/users/outil/dsx/cctools/lib-linux
LD_SCASS=-L/users/outil/dsx/systemcass/lib
OPT=-m32


all:simulator.x simulator_complex.x

#list = fifo_gcd_master.o \
		#fifo_gcd_coprocessor.o \
		#tp1_top.o
#fast_list = fast_fifo_gcd_master.o \
		#fast_fifo_gcd_coprocessor.o \
		#fast_tp1_top.o
tp2_top_list = mapping_table.o \
		segment.o \
		address_decoding_table.o \
		address_masking_table.o \
		vci_gcd_coprocessor.o \
		vci_gcd_master.o \
		tp2_simple_top.o

tp2_complex_top_list = mapping_table.o \
		segment.o \
		address_decoding_table.o \
		address_masking_table.o \
		vci_gcd_coprocessor.o \
		vci_gcd_master.o \
		vci_vgsb.o \
		tp2_complex_top.o



simulator.x: $(tp2_top_list)
	$(CC) $(CFLAG) $(PERM) $(OPT) -L. $(LD_SCASS) -o $@ $^ -lsystemc -ldl 2>&1
%.o : %.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC_SCASS) $(OPT) -c $< -o $@

simulator_complex.x: $(tp2_complex_top_list)
	$(CC) $(CFLAG) $(PERM) $(OPT) -L. $(LD_SCASS) -o $@ $^ -lsystemc -ldl 2>&1
%.o : %.cpp
	$(CC) $(CFLAG) $(PERM) $(CFLAG_OPT) -I. $(INC_SCASS) $(OPT) -c $< -o $@


clean:
	rm -f *.o *.bin core *~ *.x *.log simulation.txt
