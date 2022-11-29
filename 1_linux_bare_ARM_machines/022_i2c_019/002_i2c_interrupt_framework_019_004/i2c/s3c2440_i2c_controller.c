
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

void i2c_interrupt_func(void)
{
	/* 每传输一个数据将生产一个中断 */	


	/* 对于每次传输，第1个中断是" 已经发出了设备地址 " */	

	
}



void do_master_tx(p_i2c_msg p_msg)
{
	p_msg ->cnt_transferred = 0;
	/*设置寄存器启动传输*/
	/* 1. Master Tx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (0 << 0);/*最低位是0*/
	/* 3. Write 0xF0 (M/T Start) to IICSTAT	 */
  	IICSTAT = 0xf0; 

	/*后续的传输由中断完成 */


	/*等待中断处理完成*/
	while(p_msg ->cnt_transferred != p_msg ->len);
}

void do_master_rx(p_i2c_msg p_msg)
{
	p_msg ->cnt_transferred = 0;
	/*设置寄存器启动传输*/
	/* 1. Master Rx mode has been configured. */
	/* 2. Write slave address to IICDS. */
	IICDS = (p_msg->addr << 1) | (1 << 0);/*最低位是1*/
	/* 3. Write 0xB0 (M/R Start) to IICSTAT.  */
	IICSTAT = 0xB0; 

	/*后续的传输由中断完成 */


	/*等待中断处理完成*/
	while(p_msg ->cnt_transferred != p_msg ->len);
}


int s3c2440_master_xfer(p_i2c_msg p_msg, int num)
{
	int i;

	for(i = 0; i < num; i++)
		{
			if(0 == p_msg[i]->flag)/* write */
				do_master_tx(p_msg[i]);
			else/* read */
				do_master_rx(p_msg[i]);
		}
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
	IICCON =  (0 << 6) | (1 << 5) | (30 << 0);

	register_irq(27, i2c_interrupt_func);
}


void s3c2440_i2c_con_addr(void)
{
	register_i2c_controller(&s3c2440_i2c_con);
}




