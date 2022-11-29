
#include "i2c_controller.h"
#include "../s3c2440_soc.h"

/*ʵ��ר��оƬ�е�i2c_controller
 	.int
	.master_xfer
	.name
 */
static i2c_controller s3c2440_i2c_con = {

	.name = "s3c2440",
	.init = s3c2440_i2c_controller,
	.master_xfer = s3c2440_master_xfer,
};

static p_i2c_msg p_cur_msg;

void i2c_interrupt_func(void)
{
	int index;
	unsigned int iicstat = IICSTAT;
	
	/* ÿ����һ�����ݽ�����һ���ж� */	
	p_cur_msg->cnt_transferred++;

	/* ����ÿ�δ��䣬��1���ж���" �Ѿ��������豸��ַ " */	
	if(0 == p_cur_msg->flag)/* write */
		{
			/*���ڵ�1�жϣ��Ƿ����豸��ַ������� */
			/*
			 Ҫ�ж��Ƿ���ACK:
			     ��ACK : �豸����
   			     ��ACK : �豸�����ڣ�����ֱ�ӽ�������
			 */
			if(p_cur_msg->cnt_transferred == 0)/*��1�ν��ж�*/
				{
					if(iicstat & (1 << 0))/*1: Last-received bit is 1 (ACK was not received).*/
						{
							//stop
							/*Write 0xD0 (M/T Stop) to IICSTAT.*/
							IICSTAT = 0xD0;
							IICCON &= ~(1 << 4);
							p_cur_msg->err = -1;
							/*Wait until the stop condition takes effect.*/
							delay(1000);
							return;
						}	
				}
			
			/*���������жϣ�Ҫ����������һ������*/
			if(p_cur_msg->cnt_transferred < p_cur_msg->len)
				{
					IICDS = p_cur_msg->buf[p_cur_msg->cnt_transferred];/* Write new data transmitted to IICDS. */
					IICCON &= ~(1 << 4);/*Clear pending bit to resume.*/
				}
			else
				{	//stop
					/*Write 0xD0 (M/T Stop) to IICSTAT.*/
					IICSTAT = 0xD0;
					IICCON &= ~(1 << 4);
					p_cur_msg->err = -1;
					/*Wait until the stop condition takes effect.*/
					delay(1000);
				}
			
		}
	else/* read */
		{
			/*���ڵ�1�жϣ��Ƿ����豸��ַ������� */
			/*
			 Ҫ�ж��Ƿ���ACK:
			     ��ACK : �豸����, �ָ�I2C����, ��������һ���жϲſ��Եõ���1������
   			     ��ACK : �豸�����ڣ�����ֱ�ӽ�������
			 */
			 if(p_cur_msg->cnt_transferred == 0)/*��1�ν��ж�*/
				{
					if(iicstat & (1 << 0))/*1: Last-received bit is 1 (ACK was not received).*/
						{
							//stop
							/*Write 0x90 (M/R Stop) to IICSTAT..*/
							IICSTAT = 0x90;
							IICCON &= ~(1 << 4);
							p_cur_msg->err = -1;
							/*Wait until the stop condition takes effect.*/
							delay(1000);
							return;
						}	
					else  /* ack */
						{
							/* �ָ�I2C���� */
							IICCON &= ~(1<<4);
							return;
						}
				}


			/* �ǵ�1���ж�, ��ʾ�õ���һ��������
		 	 * ��IICDS����������
		 	 */
			if(p_cur_msg->cnt_transferred < p_cur_msg->len)
				{	
					index = p_cur_msg->cnt_transferred - 1;
					p_cur_msg->buf[index] = IICDS;
					/* �ָ�I2C���� */
					IICCON &= ~(1<<4);
				}
			else
				{
					IICSTAT = 0x90;
					IICCON &= ~(1<<4);
					p_cur_msg->err = -1;
					/*Wait until the stop condition takes effect.*/
					delay(1000);
				}
		}
}



int do_master_tx(p_i2c_msg p_msg)
{
	p_cur_msg = p_msg;
	p_msg ->cnt_transferred = -1;
	p_msg->err = 0;
	/*���üĴ�����������*/
	/* 1. Master Tx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (0 << 0);/*���λ��0*/
	/* 3. Write 0xF0 (M/T Start) to IICSTAT	 */
  	IICSTAT = 0xf0; 

	/*�����Ĵ������ж���� */


	/*�ȴ��жϴ������*/
	while(!p_msg->err && (p_msg ->cnt_transferred != p_msg ->len));

	if(p_msg->err)
		return -1;
	else
		return 0;				
}

void do_master_rx(p_i2c_msg p_msg)
{
	p_cur_msg = p_msg;
	p_msg ->cnt_transferred = -1;
	p_msg->err = 0;
	
	/*���üĴ�����������*/
	/* 1. Master Rx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (1 << 0);/*���λ��1*/
	/* 3. Write 0xB0 (M/R Start) to IICSTAT.  */
	IICSTAT = 0xB0; 

	/*�����Ĵ������жϺ������ */


	/*�ȴ��жϴ������*/
	while(!p_msg->err && (p_msg ->cnt_transferred != p_msg ->len));

	if(p_msg->err)
		return -1;
	else 
		return 0;
}


int s3c2440_master_xfer(p_i2c_msg p_msg, int num)
{
	int i;
	int err;

	for(i = 0; i < num; i++)
		{
			if(0 == p_msg[i]->flag)/* write */
				err = do_master_tx(p_msg[i]);
			else/* read */
				err = do_master_rx(p_msg[i]);

			if(err)
				return err;
		}
	
	return 0;
}

void s3c2440_i2c_con_init(void)
{
	/*����ʱ��*/
	/*
		IICCON:
		[7]: IIC-bus acknowledge enable bit.
			0: Disable
			1: Enable
		[6]: Source clock of IIC-bus transmit clock prescaler
			 selection bit.
			0: IICCLK = fPCLK /16
			1: IICCLK = fPCLK /512
		[5]: IIC-Bus Tx/Rx interrupt enable/disable bit.
			0: Disable, 
			1: Enable
		[4]:IIC-bus Tx/Rx interrupt pending flag.To resume the
			operation, clear this bit as 0
			0: 1) No interrupt pending (when read)
			1: 1) Interrupt is pending (when read)
		[3:0]:IIC-Bus transmit clock prescaler
			  Tx clock = IICCLK/(IICCON[3:0]+1)
	 */
	IICCON = (1 << 7)  | (0 << 6) | (1 << 5) | (30 << 0);

	register_irq(27, i2c_interrupt_func);
}


void s3c2440_i2c_con_addr(void)
{
	register_i2c_controller(&s3c2440_i2c_con);
}




