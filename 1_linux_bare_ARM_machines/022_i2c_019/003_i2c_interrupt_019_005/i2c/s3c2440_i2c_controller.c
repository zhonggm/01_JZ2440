
#include "i2c_controller.h"
#include "../s3c2440_soc.h"

/*实现专用芯片中的i2c_controller
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
	
	/* 每传输一个数据将生产一个中断 */	
	p_cur_msg->cnt_transferred++;

	/* 对于每次传输，第1个中断是" 已经发出了设备地址 " */	
	if(0 == p_cur_msg->flag)/* write */
		{
			/*对于第1中断，是发出设备地址后产生的 */
			/*
			 要判断是否有ACK:
			     有ACK : 设备存在
   			     无ACK : 设备不存在，出错，直接结束传输
			 */
			if(p_cur_msg->cnt_transferred == 0)/*第1次进中断*/
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
			
			/*对于其它中断，要继续发送下一个数据*/
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
			/*对于第1中断，是发出设备地址后产生的 */
			/*
			 要判断是否有ACK:
			     有ACK : 设备存在, 恢复I2C传输, 这样在下一个中断才可以得到第1个数据
   			     无ACK : 设备不存在，出错，直接结束传输
			 */
			 if(p_cur_msg->cnt_transferred == 0)/*第1次进中断*/
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
							/* 恢复I2C传输 */
							IICCON &= ~(1<<4);
							return;
						}
				}


			/* 非第1个中断, 表示得到了一个新数据
		 	 * 从IICDS读出、保存
		 	 */
			if(p_cur_msg->cnt_transferred < p_cur_msg->len)
				{	
					index = p_cur_msg->cnt_transferred - 1;
					p_cur_msg->buf[index] = IICDS;
					/* 恢复I2C传输 */
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
	/*设置寄存器启动传输*/
	/* 1. Master Tx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (0 << 0);/*最低位是0*/
	/* 3. Write 0xF0 (M/T Start) to IICSTAT	 */
  	IICSTAT = 0xf0; 

	/*后续的传输由中断完成 */


	/*等待中断处理完成*/
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
	
	/*设置寄存器启动传输*/
	/* 1. Master Rx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (1 << 0);/*最低位是1*/
	/* 3. Write 0xB0 (M/R Start) to IICSTAT.  */
	IICSTAT = 0xB0; 

	/*后续的传输由中断函数完成 */


	/*等待中断处理完成*/
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
	/*设置时钟*/
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




