#include "stdio.h"

#define	TRUE	1
#define	FALSE	0

__attribute__((constructor)) void main()
{
    unsigned    prime[1000];
    unsigned    tested_value = 2;
    unsigned    next_empty_slot = 0;
    unsigned    is_prime;
    unsigned    i;
    char	c;

    tty_printf("*** Starting Prime Computation ***\n\n");

    while (1)
    {
        is_prime = TRUE;
        for( i=0 ; i<next_empty_slot ; i++ )
        {
            if( tested_value%prime[i] == 0 ) is_prime = FALSE;
        }
        if ( is_prime )
        {
            prime[next_empty_slot] = tested_value;
            tty_printf("prime[%d] = %d\n", next_empty_slot, tested_value);
            next_empty_slot++;
            if( next_empty_slot == 1000)
            {
                tty_printf("prime table full\n");
                exit();
            }
        }
        tested_value++;

	tty_getc_irq((char*)&c);

        if( c == 'q' ) exit();
    }
} // end main
