
void delay(volatile int d)
{
	while(d--);
}



int led_on(int led_number)
{
	unsigned int *pGPFCON = (unsigned int *)0x56000050;
	unsigned int *pGPFDAT = (unsigned int *)0x56000054;

	if(led_number == 4)
		*pGPFCON = 0x100;
	else if(led_number == 5)
		*pGPFCON = 0x400;

	*pGPFDAT = 0;

	return 0;
}



