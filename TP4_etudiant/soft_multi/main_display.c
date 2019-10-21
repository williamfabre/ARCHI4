#include "stdio.h"
#define LBA 

#define NB_PIXELS 128
#define NB_LINES 128
#define BLOCK_SIZE 512



__attribute__((constructor)) void main_display()
{
	char buf_in[NB_LINES*NB_PIXELS];
	char byte;
	int base = 0;
	int image = 0;
	int pid = procid();
	int nprocs = procnumber();
	int npixels;
	int nblocks;

	while(image < 20)
	{
		tty_puts("my little program display \n");

		npixels = NB_PIXELS*NB_LINES;
		nblocks = npixels/BLOCK_SIZE;

		ioc_read(base , buf_in, nblocks);

		if(ioc_completed())
		{
			tty_printf("ERROR : ioc_completed syscall au cycle %d\n", proctime());
		}

		if (fb_write(0, buf_in, NB_PIXELS*NB_LINES) != 0)
		{
			tty_printf("ERROR : fb_write syscall au cycle %d\n", proctime());
		}

		base = base + nblocks;
		image = image + 1;
		tty_getc_irq(&byte);
	}
	exit();
} // end main
