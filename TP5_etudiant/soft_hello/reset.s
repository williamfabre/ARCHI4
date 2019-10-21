#################################################################################
#	File : reset.s
#	Author : Alain Greiner
#	Date : 20/12/2013
#################################################################################
# 	This is a boot code for a clusterized architecture:
#       4 clusters / 1 processor per cluster. 
#	- Each processor initializes the Status Register (SR) 
#       - There is one stack per cluster, and each processor computes 
#         the stack base address as seg_stack_base + proc_id*0x10000000
#	- All processors initialize concurrently the Interrupt Vector.
#       - Each processor initializes its private ICU channel mask.
#	- Each processor P[i] initializes its EPC register, and jumps to
#         the user code entry point.
#################################################################################
		
	.section .reset,"ax",@progbits

	.extern	seg_stack_base
	.extern	seg_data_base
	.extern	seg_icu_base
	.extern _interrupt_vector
	.extern _isr_timer
	.extern _isr_tty_get
	.extern _isr_dma
	.extern _isr_ioc

	.globl  reset	 			# makes reset an external symbol 
	.ent	reset
	.align	2

reset:
       	.set noreorder

    # Each processor initializes SR register
    li	    $26,    0x0000FF13		
    mtc0    $26,    $12			# SR <= user mode / IRQ enable (after eret)

    # Each procesor initializes stack pointer depending on the proc_id
    la      $27,    seg_stack_base
    mfc0    $26,    $15,    1
    andi    $10,    $26,    0x3		# $10 <= proc_id
    sll     $11,    $10,    28		# $11 <= proc_id * 0x10000000
    addu    $27,    $27,    $11		# $27 <= seg_stack_base + proc_id*0x10000000 
    li	    $26,    0x10000		# $26 <= 64K
    addu    $29,    $27, $26		# $29 <= seg_stack_base + proc_id*0x10000000 + 64K

    # All processors initialize concurrently interrupt vector: 13 IRQs
    la      $26,    TO BE COMPLETED     # interrupt vector address
    la      $27,    TO BE COMPLETED     # isr_timer address
    sw      $27,    0($26)              # interrupt_vector[0] <= _isr_timer
    sw      $27,    4($26)              # interrupt_vector[1] <= _isr_timer
    sw      $27,    8($26)              # interrupt_vector[2] <= _isr_timer
    sw      $27,   12($26)              # interrupt_vector[3] <= _isr_timer
    la      $27,    TO BE COMPLETED     # isr_dma address
    sw      $27,   16($26)              # interrupt_vector[4] <= _isr_dma  
    sw      $27,   20($26)              # interrupt_vector[5] <= _isr_dma  
    sw      $27,   24($26)              # interrupt_vector[6] <= _isr_dma  
    sw      $27,   28($26)              # interrupt_vector[7] <= _isr_dma  
    la      $27,    TO BE COMPLETED     # isr_tty address
    sw      $27,   32($26)              # interrupt_vector[8]  <= _isr_tty_get
    sw      $27,   36($26)              # interrupt_vector[9]  <= _isr_tty_get
    sw      $27,   40($26)              # interrupt_vector[10] <= _isr_tty_get
    sw      $27,   44($26)              # interrupt_vector[11] <= _isr_tty_get
    la      $27,    TO BE COMPLETED     # isr_ioc address
    sw      $27,   48($26)              # interrupt_vector[12] <= _isr_ioc

    # Each processor computesizes ICU imask address, depending on the proc_id
    sll     $26,    $10,    5           # $26 <= proc_id*32
    la      $27,    seg_icu_base
    addu    $12,    $26,    $27		# $12 <= seg_icu_base + proc_id*0x32

reset_proc0:
    # Proc 0 set ICU_MASK[0]
    bne     $10,    $0,     reset_proc1
    li      $26,    TO BE COMPLETED     # IOC[0] TTY[0] DMA[0] TIM[0]
    sw      $26,    8($12)              # ICU_MASK_SET

    # proc 0 jumps to user code
    la      $26,    seg_data_base       # $26 <= &start_vector[0]
    li      $27,    0                   # $27 <= index*4
    addu    $26,    $26,    $27         # $26 <= &start_vector[index]
    lw      $26,    0($26)              # $26 <= start_vector[index]
    mtc0    $26,    $14			# EPC <= start_vector[index]
    eret

reset_proc1:
    # Proc 1 set ICU_MASK[1]
    li      $26,    1
    bne     $10,    $26,   reset_proc2
    li      $26,    TO BE COMPLETED     # TTY[1] DMA[1] TIM[1]
    sw      $26,    8($12)              # ICU_MASK_SET

    # proc 1 jumps to user code
    la      $26,    seg_data_base       # $26 <= &start_vector[0]
    li      $27,    0                   # $27 <= index*4
    addu    $26,    $26,    $27         # $26 <= &start_vector[index]
    lw      $26,    0($26)              # $26 <= start_vector[index]
    mtc0    $26,    $14			# EPC <= start_vector[index]
    eret

reset_proc2:
    # Proc 2 set ICU_MASK[2]
    li      $26,    2
    bne     $10,    $26,   reset_proc3
    li      $26,    TO BE COMPLETED     # TTY[2] DMA[2] TIM[2]
    sw      $26,    8($12)              # ICU_MASK_SET

    # proc 2 jumps to user code
    la      $26,    seg_data_base       # $26 <= &start_vector[0]
    li      $27,    0                   # $27 <= index*4
    addu    $26,    $26,    $27         # $26 <= &start_vector[index]
    lw      $26,    0($26)              # $26 <= start_vector[index]
    mtc0    $26,    $14			# EPC <= start_vector[index]
    eret

reset_proc3:
    # Proc 3 set ICU_MASK[3]
    li      $26,    TO BE COMPLETED     # TTY[3] DMA[3] TIM[3]
    sw      $26,    8($12)              # ICU_MASK_SET

    # proc 3 jumps to user code
    la      $26,    seg_data_base       # $26 <= &start_vector[0]
    li      $27,    0                   # $27 <= index*4
    addu    $26,    $26,    $27         # $26 <= &start_vector[index]
    lw      $26,    0($26)              # $26 <= start_vector[index]
    mtc0    $26,    $14			# EPC <= start_vector[index]
    eret

    .end	reset

    .set reorder
