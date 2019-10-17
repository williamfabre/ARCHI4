#include "stdio.h"

__attribute__((constructor)) void main()
{
	char  byte;
	/*int it=0;*/
	/*int res = -1;*/
	/*int status = 0;*/

	while(1)
	{
		tty_puts("my little program");
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
