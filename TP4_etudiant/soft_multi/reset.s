#################################################################################
#	File : reset.s
#	Author : Alain Greiner
#	Date : 28/11/2013
#################################################################################
#	- It initializes the Interrupt vector
#	- It initializes the Status Register (SR) 
#	- It defines the stack size  and initializes the stack pointer ($29) 
#	- It initializes the EPC register, and jump to the user code    
#################################################################################
		
	.section .reset,"ax",@progbits

	.extern	seg_stack_base
	.extern	seg_data_base
	.extern	seg_icu_base
	.extern _interrupt_vector
	.extern _isr_timer
	.extern _isr_tty_get_task0
	.extern _isr_tty_get_task1
	.extern _isr_tty_get_task2
	.extern _isr_tty_get_task3
	.extern _isr_dma
	.extern _isr_ioc

	.globl  reset	 			# makes reset an external symbol 
	.ent	reset
	.align	2

reset:
       	.set noreorder
# get the processor id and branch
        mfc0	$27,	$15,	1		# get the proc_id
        andi	$27,	$27,	0x3		# no more than 4 processors
        li      $26,	0
        beq     $27,	$26,	proc0
        li      $26,	1
        beq     $27,	$26,	proc1
        li      $26,	2
        beq     $27,	$26,	proc2
        li      $26,	3
        beq     $27,	$26,	proc3
        nop

proc0:
        # initialises interrupt vector entries for PROC[0]
        la      $26,	_interrupt_vector
        #la      $27,	_isr_ioc
        #sw      $27,	0($26)
        la      $27,	_isr_dma
        sw      $27,	4*4($26)
        la      $27,	_isr_timer
        sw      $27,	4*8($26)
        la      $27,	_isr_tty_get_task0
        sw      $27,	4*12($26)

        #initializes the ICU[0] MASK register
        la      $26,	seg_icu_base
        addiu	$26,	$26,	0
        li      $27,	0x1110
        sw      $27,	8($26)

        # initializes stack pointer for PROC[0]
        la      $29,	seg_stack_base
        li      $27,	0x10000
        addu	$29,	$29,	$27

        # initializes SR register for PROC[0]
        li      $26,	0x0000FF13
       	mtc0	$26,	$12

        # jump to main in user mode: main[0]
        la      $26,	seg_data_base
        lw      $26,	0($26)
        mtc0	$26,	$14
        eret

proc1:
        # initialises interrupt vector entries for PROC[1]
        la      $26,	_interrupt_vector
		#la      $27,	_isr_ioc
		#sw      $27,	0($26)
        la      $27,	_isr_dma
        sw      $27,	4*5($26)
        la      $27,	_isr_timer
        sw      $27,	4*9($26)
        la      $27,	_isr_tty_get_task0
        sw      $27,	4*13($26)

        #initializes the ICU[1] MASK register
        la      $26,	seg_icu_base
        addiu	$26,	$26,	32
        li      $27,	0x2220
        sw      $27,	8($26)

        # initializes stack pointer for PROC[1]
        la      $29,	seg_stack_base
        li      $27,	0x20000         # TO BE COMPLETED		 stack size = 64K
        addu	$29,	$29,	$27    	# $29 <= seg_stack_base + 128K

        # initializes SR register for PROC[1]
       	li      $26,	0x0000FF13	
       	mtc0	$26,	$12			    # SR <= 0x0000FF13

        # jump to main in user mode: main[1]
        la      $26,	seg_data_base
        lw      $26,	0($26)			# $26 <= main[1] 
        mtc0	$26,	$14			    # write it in EPC register
        eret

proc2:
        # initialises interrupt vector entries for PROC[2]
        la      $26,	_interrupt_vector
		la      $27,	_isr_ioc
		sw      $27,	0($26)
        la      $27,	_isr_dma
        sw      $27,	4*6($26)
        la      $27,	_isr_timer
        sw      $27,	4*10($26)
        la      $27,	_isr_tty_get_task0
        sw      $27,	4*14($26)

        #initializes the ICU[2] MASK register
        la      $26,	seg_icu_base
        addiu	$26,	$26,	64		# ICU[2]
        li      $27,	0x4441
        sw      $27,	8($26)

        # initializes stack pointer for PROC[2]
        la      $29,	seg_stack_base
        li      $27,	0x30000         # TO BE COMPLETED		 stack size = 64K
        addu	$29,	$29,	$27    	# $29 <= seg_stack_base + 192K

        # initializes SR register for PROC[2]
       	li      $26,	0x0000FF13	
       	mtc0	$26,	$12			    # SR <= 0x0000FF13

        # jump to main in user mode: main[2]
        la      $26,	seg_data_base
        lw      $26,	4($26)			# $26 <= main[2] 
        mtc0	$26,	$14			    # write it in EPC register
        eret

proc3:
        # initialises interrupt vector entries for PROC[3]
        la      $26,	_interrupt_vector
		#la      $27,	_isr_ioc
		#sw      $27,	0($26)
        la      $27,	_isr_dma
        sw      $27,	4*7($26)
        la      $27,	_isr_timer
        sw      $27,	4*11($26)
        la      $27,	_isr_tty_get_task0
        sw      $27,	4*15($26)

        #initializes the ICU[3] MASK register
        la      $26,	seg_icu_base
        addiu	$26,	$26,	96		# ICU[3]
        li      $27,	0x8880
        sw      $27,	8($26)

        # initializes stack pointer for PROC[3]
        la      $29,	seg_stack_base
        li      $27,	0x40000         # TO BE COMPLETED		 stack size = 64K
        addu	$29,	$29,	$27    	# $29 <= seg_stack_base + 256K

        # initializes SR register for PROC[3]
       	li      $26,	0x0000FF13	
       	mtc0	$26,	$12			    # SR <= 0x0000FF13

        # jump to main in user mode: main[3]
        la      $26,	seg_data_base
        lw      $26,	0($26)			# $26 <= main[3] 
        mtc0	$26,	$14			    # write it in EPC register
        eret
	.end	reset
	.size	reset, .-reset

	.set reorder
