#include "stdio.h"

__attribute__((constructor)) void  main() // ecrit hello world puis attend un caractere en boucle
{
	char  byte;

	while(1) 
        {
	    tty_puts("\nhello world\n");
	    tty_getc(&byte);
	}

} // end main
