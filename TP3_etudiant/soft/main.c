#include "stdio.h"

/* GCD coprocessor related functions */
/*unsigned int gcd_set_opa(unsigned int val);*/
/*unsigned int gcd_set_opb(unsigned int val);*/
/*unsigned int gcd_start();*/
/*unsigned int gcd_get_result(unsigned int *val);*/
/*unsigned int gcd_get_status(unsigned int *val);*/

__attribute__((constructor)) void  main() // ecrit hello world puis attend un caractere en boucle
{
	char  byte;
	int it=0;
	int res = -1;
	int status = 0;

	while(1)
	{
		// declaration de variable dans la portee
		/*int i;*/
		tty_printf("time START is : %d ", proctime());
		tty_printf("cycle is : %d\n", it++);
		int opa = (rand()+1)%100;
		int opb = (rand()+1)%100;
		gcd_set_opa((unsigned int)opa);
		gcd_set_opb((unsigned int)opb);
		gcd_start();

		while (status != 0)
			gcd_get_status(&status);

		gcd_get_result(&res);

		tty_printf("time END is : %d ", proctime());
		tty_printf("pgcd(%d,%d)=%d",opa, opb, res);
		tty_printf("cycle is : %d\n", it++);
		tty_printf("\n");
		tty_printf("\n");
		tty_printf("\n");


		tty_getc(&byte);


		/*tty_puts("\nhello world\n");*/
	}

} // end main
