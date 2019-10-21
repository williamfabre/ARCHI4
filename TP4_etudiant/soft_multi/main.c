#include "stdio.h"

__attribute__((constructor)) void main()
{
	char  byte;

	while(1)
	{
		tty_puts("my little program\n");
		tty_puts("a set timer\n");
		tty_puts("d unset timer\n");
		tty_printf(" PROCID : %d ", procid());
		tty_puts("q exit\n");
		timer_set_period(500000);

		tty_getc_irq(&byte);
		switch(byte)
		{
		case 'a':
			timer_set_mode(0x3);
			break;
		case 'd':
			timer_set_mode(0x1);
			break;
		case 'q':
			exit();
			break;
		default:
			break;
		}
	}
} // end main
