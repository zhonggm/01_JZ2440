

#include "s3c2440_soc.h"
#include "interrupt.h"

irq_func irq_array[32];

/***
SRCPND
������ʾ�ĸ��жϲ����ˣ���Ҫ�����Ӧλ,д1���
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/

/***
INTMOD 
���������ж�Ϊ�����ж�(0)���ǿ��ж�(1)
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/

/***
INTMSK
���������жϿɱ�����(0)���ǲ��ɱ�����(1)
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
PRIORITY
��������
***/


/***
INTPND
������ʾ��ǰ���ȼ���ߵġ����ڷ�����(1)Ψһ�ж����ĸ�����Ҫ�����Ӧλ��д1���
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
INTOFFSET
������ʾINTPND����һλ����1
bit5 - eint8-23
bit2 - eint2
bit0 - eint0
***/


/***
EINTPEND
������ʾEINT22 - EINT4����һ�жϱ�������1����д1���
bit11 - eint11
bit9  - eint9
***/

/* �ж�ע�ắ�� */
void register_irq(int irq, irq_func fp)
{
	irq_array[irq] = fp;/*���жϺ����ŵ�������*/
	INTMSK &= ~(1<<irq);
}


/* ��ʼ���жϿ����� */
void interrupt_init(void) 
{
	INTMSK &= ~((1<<0) | (1<<2) | (1<<5));/*enable eint0/eint2/eint11/eint19 interrupt*/
	INTMSK &= ~(1<<10);/*enable timer0 interrupt*/
}

/* ��ȡEINTPEND���ֱ��ĸ��жϷ����� */
void key_eint_irq(int int_offset)
{
	unsigned int eintpend_val = EINTPEND;
	unsigned int key_s2_s3_level = GPFDAT;
	unsigned int key_s4_s5_level    = GPGDAT;

	if(int_offset == 0)/* EINT0 �� ���� D12 */
		{
			if(key_s2_s3_level & (1<<0))/* S2-GPF0����GPF6 D12*/
			{//�ɿ�
				GPFDAT |= (1<<6);
			}
			else
			{//����
				GPFDAT &= ~(1<<6);
			}
			
		}
	else if(int_offset == 2)/* EINT2 */
		{
			if(key_s2_s3_level & (1<<2))/* S3- GPF2����GPF5 - D11 */
			{//�ɿ�
				GPFDAT |= (1<<5);
			}
			else
			{//����
				GPFDAT &= ~(1<<5);
			}
		}
	else if(int_offset == 5)/* EINT5 */
		{
			if(eintpend_val & (1<<11))/* EINT11-GPG3 - s4 ���� GPF4  D10*/
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
			else if(eintpend_val & (1<<19))/* EINT19-GPG11-s5���� GPF6 GPF5 GPF4 */
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


/* ��ʼ����������Ϊ�ж�Դ */
void key_eint_init(void) 
{
	/*************
	EINT0  - S2 - GPF0 
	EINT2  - S3 - GPF2  
	EINT11 - S4 - GPG3 
	EINT19 - S5 - GPG11 
	*************/
	/* ����GFIOΪ�ж�����  ֮ GPF0 �� GPF2 */
	GPFCON &= ~((3<<0) | (3<<4));
	GPFCON |=  ((2<<0) | (2<<4));
	
	/* ����GFIOΪ�ж�����  ֮ GPG3 �� GPG11 */
	GPGCON &= ~((3<<6) | (3<<22));
	GPGCON |=  ((2<<6) | (2<<22));


	/*�����ж� EINT0 EINT2 ����Ϊ ˫���ش���*/
	EXTINT0 |= ((7 << 0) | (7 << 8));
	EXTINT1 |= (7 << 12);/* EINT11 Ϊ˫���� */
	EXTINT2 |= (7 << 12);/* EINT19 Ϊ˫���� */


	/* ʹ��EINT11 EINT19��EINT0 EINT2��ʹ�ܲ������� */
	EINTMASK &= ~((1<<11) | (1<<19));


	register_irq(0, key_eint_irq);
	register_irq(2, key_eint_irq);
	register_irq(5, key_eint_irq);

}




void handl_irq_c(void)
{
	/*�ֱ��ж�Դ*/
	int bit = INTOFFSET;

	/*���ö�Ӧ���жϺ���*/
	irq_array[bit](bit);
	/*���жϣ�*/
	SRCPND = (1<<bit);
	INTPND = (1<<bit);
}







