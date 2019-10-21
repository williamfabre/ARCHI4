#include "stdio.h"

__attribute__((constructor)) void main()
{
    volatile char	c;

    while(1) 
    {
        tty_puts("hello world\n");
        tty_getc_irq( (void*)&c );
    }
} // end main
