

#include "s3c2440_soc.h"
#include "interrupt.h"

irq_func irq_array[32];

/***
SRCPND
用来表示哪个中断产生了，需要清除对应位,写1清除
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/

/***
INTMOD 
用来设置中断为正常中断(0)还是快中断(1)
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/

/***
INTMSK
用来设置中断可被服务(0)还是不可被服务(1)
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
PRIORITY
不用设置
***/


/***
INTPND
用来表示当前优先级最高的、正在发生的(1)唯一中断是哪个，需要清除对应位，写1清除
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
INTOFFSET
用来表示INTPND中哪一位被置1
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
EINTPEND
用来表示EINT22 - EINT4中哪一中断被挂起（置1），写1清除
bit11 - eint11
bit9  - eint9
***/

/* 中断注册函数 */
void register_irq(int irq, irq_func fp)
{
	irq_array[irq] = fp;/*把中断函数放到数组中*/
	INTMSK &= ~(1<<irq);
}


/* 初始化中断控制器 */
void interrupt_init(void) 
{
	INTMSK &= ~((1<<0) | (1<<2) | (1<<5));/*enable eint0/eint2/eint11/eint19 interrupt*/
	INTMSK &= ~(1<<10);/*enable timer0 interrupt*/
}

/* 读取EINTPEND来分辨哪个中断发生了 */
void key_eint_irq(int int_offset)
{
	unsigned int eintpend_val = EINTPEND;
	unsigned int key_s2_s3_level = GPFDAT;
	unsigned int key_s4_s5_level    = GPGDAT;

	if(int_offset == 0)/* EINT0 ， 控制 D12 */
		{
			if(key_s2_s3_level & (1<<0))/* S2-GPF0控制GPF6 D12*/
			{//松开
				GPFDAT |= (1<<6);
			}
			else
			{//按下
				GPFDAT &= ~(1<<6);
			}
			
		}
	else if(int_offset == 2)/* EINT2 */
		{
			if(key_s2_s3_level & (1<<2))/* S3- GPF2控制GPF5 - D11 */
			{//松开
				GPFDAT |= (1<<5);
			}
			else
			{//按下
				GPFDAT &= ~(1<<5);
			}
		}
	else if(int_offset == 5)/* EINT5 */
		{
			if(eintpend_val & (1<<11))/* EINT11-GPG3 - s4 控制 GPF4  D10*/
			{
				if(key_s4_s5_level & (1<<3))
					{
						GPFDAT |= (1<<4);
					}
				else
					{
						GPFDAT &= ~(1<<4);						
					}
			}
			else if(eintpend_val & (1<<19))/* EINT19-GPG11-s5控制 GPF6 GPF5 GPF4 */
			{
				if(key_s4_s5_level & (1<<11))
					{
						GPFDAT |= ((1<<6)|(1<<5)|(1<<4));
					}
				else
					{
						GPFDAT &= ~((1<<6)|(1<<5)|(1<<4));						
					}
			}
		}

	EINTPEND = eintpend_val;
}


/* 初始化按键，设为中断源 */
void key_eint_init(void) 
{
	/*************
	EINT0  - S2 - GPF0 
	EINT2  - S3 - GPF2  
	EINT11 - S4 - GPG3 
	EINT19 - S5 - GPG11 
	*************/
	/* 配置GFIO为中断引脚  之 GPF0 和 GPF2 */
	GPFCON &= ~((3<<0) | (3<<4));
	GPFCON |=  ((2<<0) | (2<<4));
	
	/* 配置GFIO为中断引脚  之 GPG3 和 GPG11 */
	GPGCON &= ~((3<<6) | (3<<22));
	GPGCON |=  ((2<<6) | (2<<22));


	/*设置中断 EINT0 EINT2 触发为 双边沿触发*/
	EXTINT0 |= ((7 << 0) | (7 << 8));
	EXTINT1 |= (7 << 12);/* EINT11 为双边沿 */
	EXTINT2 |= (7 << 12);/* EINT19 为双边沿 */


	/* 使能EINT11 EINT19，EINT0 EINT2的使能不用设置 */
	EINTMASK &= ~((1<<11) | (1<<19));


	register_irq(0, key_eint_irq);
	register_irq(2, key_eint_irq);
	register_irq(5, key_eint_irq);

}




void handl_irq_c(void)
{
	/*分辨中断源*/
	int bit = INTOFFSET;

	/*调用对应的中断函数*/
	irq_array[bit](bit);
	/*清中断，*/
	SRCPND = (1<<bit);
	INTPND = (1<<bit);
}







