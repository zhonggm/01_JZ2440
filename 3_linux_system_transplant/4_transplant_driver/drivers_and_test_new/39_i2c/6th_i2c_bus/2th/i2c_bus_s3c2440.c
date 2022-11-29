#include <linux/kernel.h>
#include <linux/module.h>//THIS_MODULE声明定义

#include <linux/i2c.h>//I2C_M_RD定义
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>//包含irq_handler_t定义

#include <linux/delay.h>//ndelay声明定义
#include <linux/errno.h>
#include <linux/err.h>

#include <linux/platform_device.h>
#include <linux/pm_runtime.h>


#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_i2c.h>
#include <linux/of_gpio.h>

#include <asm/irq.h>
#include <plat/regs-iic.h>
#include <plat/iic.h>

#include <plat/gpio-cfg.h>//S3C2410_GPE14_IICSCL定义
#include <mach/regs-gpio.h>


//#define PRINTK printk
#define PRINTK(...) 

//定义i2c的状态枚举
enum s3c24xx_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct s3c2440_i2c_regs{
	unsigned int iiccon;
	unsigned int iicstat;
	unsigned int iicadd;
	unsigned int iicds;
	unsigned int iiclc;	
};
static struct s3c2440_i2c_regs *s3c2440_i2c_regs;

//定义用于保存数据的结构体类型s3c2440_i2c_xfer_data及变量
struct s3c2440_i2c_xfer_data {
	struct i2c_msg *msgs;
	int msn_num;
	int cur_msg;
	int cur_ptr;
	int state;
	int err;
	wait_queue_head_t wait;
};
static struct s3c2440_i2c_xfer_data s3c2440_i2c_xfer_data;

static void s3c2440_i2c_start(void)
{
	s3c2440_i2c_xfer_data.state = STATE_START;//置位状态

	/* 读操作 */
	if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ 
		s3c2440_i2c_regs->iicds		 = s3c2440_i2c_xfer_data.msgs->addr << 1;
		s3c2440_i2c_regs->iicstat 	 = 0xb0;	// 主机接收，启动
	}else{ /* 写操作 */
		s3c2440_i2c_regs->iicds		 = s3c2440_i2c_xfer_data.msgs->addr << 1;
		s3c2440_i2c_regs->iicstat    = 0xf0; 	// 主机发送，启动
	}
}


static void s3c2440_i2c_stop(int err)
{
	s3c2440_i2c_xfer_data.state = STATE_STOP;
	s3c2440_i2c_xfer_data.err   = err;

	PRINTK("STATE_STOP, err = %d\n", err);

	/* 读操作 */
	if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ 
	
		// 下面两行恢复I2C操作，发出P信号
		s3c2440_i2c_regs->iicstat = 0x90;
		s3c2440_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // 等待一段时间以便P信号已经发出
		
	}else{ /* 写操作 */
		// 下面两行用来恢复I2C操作，发出P信号
		s3c2440_i2c_regs->iicstat = 0xd0;
		s3c2440_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // 等待一段时间以便P信号已经发出
	}

	/* 发完停止信号之后要唤醒 */
	wake_up(&s3c2440_i2c_xfer_data.wait);
}


static int s3c2440_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	unsigned long timeout;

	/* 把num个msg的I2C数据发送出去/读进来 */
	s3c2440_i2c_xfer_data.msgs    = msgs;
	s3c2440_i2c_xfer_data.msn_num = num;
	s3c2440_i2c_xfer_data.cur_msg = 0;
	s3c2440_i2c_xfer_data.cur_ptr = 0;
	s3c2440_i2c_xfer_data.err     = -ENODEV;/*置为ENODEV，等以后结束的时候会把这个err设置为0。*/
	
	s3c2440_i2c_start();

	/* 休眠 */
	/* 退出休眠条件是state 为stop  */
	timeout = wait_event_timeout(s3c2440_i2c_xfer_data.wait, (s3c2440_i2c_xfer_data.state == STATE_STOP), HZ * 5);	
	/* 判断退出原因 */
	if(0 == timeout){
		printk("s3c2440_i2c_xfer time out\n");
		return -ETIMEDOUT;
	}else{
		return s3c2440_i2c_xfer_data.err;
	}
}

static u32 s3c2440_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

static const struct i2c_algorithm s3c2440_i2c_algo = {
	.master_xfer	= s3c2440_i2c_xfer,
	.functionality	= s3c2440_i2c_func,
};

/*  1. 分配、设置i2c_adapter  */
static struct i2c_adapter s3c2440_i2c_adapter = {
 .name			 = "s3c2440_100ask",
 .algo			 = &s3c2440_i2c_algo,
 .owner 		 = THIS_MODULE,
};

/* 是最后一个消息 */
static int isLastMsg(void)
{
	return (s3c2440_i2c_xfer_data.cur_msg == s3c2440_i2c_xfer_data.msn_num - 1);
}

/* 已经是最后一个数据*/
static int isEndData(void)
{
	return (s3c2440_i2c_xfer_data.cur_ptr >= s3c2440_i2c_xfer_data.msgs->len);
}

/* 即将是最后一个数据*/
static int isLastData(void)
{
	return (s3c2440_i2c_xfer_data.cur_ptr == s3c2440_i2c_xfer_data.msgs->len - 1);
}

static irq_handler_t s3c2440_i2c_xfer_irq(unsigned int irq, void *dev_id)
{
	unsigned int iicSt;
    iicSt = s3c2440_i2c_regs->iicstat; 

	if(iicSt & 0x8){ printk("Bus arbitration failed\n\r"); }

	switch (s3c2440_i2c_xfer_data.state){
		case STATE_START : /* 发出S和设备地址后,产生中断 */
		{
			PRINTK("Start\n");
			/* 如果没有收到ACK, 停止并返回错误 */
			if (iicSt & S3C2410_IICSTAT_LASTBIT){
				s3c2440_i2c_stop(-ENODEV);
				break;
			}
			
			//如果是最后一个消息并且是最后一个数据，那么成功。
			if (isLastMsg() && isEndData()){
				s3c2440_i2c_stop(0);
				break;
			}

			/* 如果既不是没有ack，又不是最后一帧数据，进入下一个状态 */
			if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ /* 读 */
				//如果是读的话goto next_read
				s3c2440_i2c_xfer_data.state = STATE_READ;
				goto next_read;
			}else{											   /* 写 */
				/*如果是写的话就继续，不用break，
				    就继续下一个case  STATE_WRITE 执行发数据操作了
				   */
				s3c2440_i2c_xfer_data.state = STATE_WRITE;
			}	
		}
		case STATE_WRITE:
		{
			PRINTK("STATE_WRITE\n");
			/* 如果没有收到ACK, 停止并返回错误 */
			if (iicSt & S3C2410_IICSTAT_LASTBIT){
				s3c2440_i2c_stop(-ENODEV);
				break;
			}
			/* 如果当前msg还有数据要发送 */
			if (!isEndData()){  
				s3c2440_i2c_regs->iicds = s3c2440_i2c_xfer_data.msgs->buf[s3c2440_i2c_xfer_data.cur_ptr];
				s3c2440_i2c_xfer_data.cur_ptr++;
				
				// 将数据写入IICDS后，需要一段时间才能出现在SDA线上
				ndelay(50);	
				
				s3c2440_i2c_regs->iiccon = 0xaf;// 恢复I2C传输
				break;				
			}else if (!isLastMsg()){
				/* 开始处理下一个消息 */
				s3c2440_i2c_xfer_data.msgs++;
				s3c2440_i2c_xfer_data.cur_msg++;
				s3c2440_i2c_xfer_data.cur_ptr = 0;
				s3c2440_i2c_xfer_data.state = STATE_START;
				/* 发出START信号和发出设备地址 */
				s3c2440_i2c_start();
				break;
			}else{
				/* 是最后一个消息的最后一个数据就停止并不报错 */
				s3c2440_i2c_stop(0);
				break;				
			}
			break;
		}
		case STATE_READ:
		{
			PRINTK("STATE_READ\n");
			/* 
			读出数据 
			在next_read之前要先把数据读出来，一旦产生了中断直接进入
			STATE_READ的话就表明已经读到了一个数据。
			*/
			s3c2440_i2c_xfer_data.msgs->buf[s3c2440_i2c_xfer_data.cur_ptr] = s3c2440_i2c_regs->iicds;			
			s3c2440_i2c_xfer_data.cur_ptr++;
next_read:
			if (!isEndData()){ /* 如果数据没读完,  继续发起读操作 */
				if (isLastData()){  /* 如果即将读的数据是最后一个, 不发ack */
					s3c2440_i2c_regs->iiccon = 0x2f;// 恢复I2C传输，接收到下一数据时无ACK
				}else{
					s3c2440_i2c_regs->iiccon = 0xaf;// 恢复I2C传输，接收到下一数据时发出ACK
				}				
				break;
			}else if (!isLastMsg()){
				/* 开始处理下一个消息 */
				s3c2440_i2c_xfer_data.msgs++;
				s3c2440_i2c_xfer_data.cur_msg++;
				s3c2440_i2c_xfer_data.cur_ptr = 0;
				s3c2440_i2c_xfer_data.state = STATE_START;
				/* 发出START信号和发出设备地址 */
				s3c2440_i2c_start();
				break;
			}else{
				/* 是最后一个消息的最后一个数据 */
				s3c2440_i2c_stop(0);
				break;								
			}
			break;
		}
		default: break;
	}
	/* 清i2c控制器中断标志 */
	s3c2440_i2c_regs->iiccon &= ~(S3C2410_IICCON_IRQPEND);

	return IRQ_HANDLED;	
}


static void s3c2440_i2c_init(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "i2c");//找到时钟
	clk_enable(clk);           //使能时钟
	
	// 选择引脚功能：GPE15:IICSDA, GPE14:IICSCL
	s3c_gpio_cfgpin(S3C2410_GPE(14), S3C2410_GPE14_IICSCL);
	s3c_gpio_cfgpin(S3C2410_GPE(15), S3C2410_GPE15_IICSDA);

   /* bit[7]  = 1, 使能ACK
	 * bit[6] = 0, IICCLK = PCLK/16
	 * bit[5] = 1, 使能中断
	 * bit[3:0] = 0xf, Tx clock = IICCLK/16
	 * PCLK = 50MHz, IICCLK = 3.125MHz, Tx Clock = 0.195MHz
	 */
	s3c2440_i2c_regs->iiccon = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf
	s3c2440_i2c_regs->iicadd  = 0x10;	  // S3C24xx slave address = [7:1]
	s3c2440_i2c_regs->iicstat = 0x10;	  //使能 I2C串行输出(Rx/Tx)
}

static int i2c_bus_s3c2440_init(void)
{
	/* 2. 硬件相关的设置 *///从物理地址到虚拟地址的映射
	s3c2440_i2c_regs = ioremap(0x54000000, sizeof(struct s3c2440_i2c_regs));

	s3c2440_i2c_init();

	/*注册中断*/
	request_irq(IRQ_IIC, s3c2440_i2c_xfer_irq, 0, "s3c2440-i2c", NULL);

	/*初始化等待队列*/
	init_waitqueue_head(&s3c2440_i2c_xfer_data.wait);
		
	/* 3. 注册适配器 i2c_adapter  */	
	i2c_add_adapter(&s3c2440_i2c_adapter);
	
	return 0;
}

static void i2c_bus_s3c2440_exit(void)
{
	/* 3. 注销适配器i2c_adapter  */	
	i2c_del_adapter(&s3c2440_i2c_adapter);
	free_irq(IRQ_IIC, NULL);
	//解除地址映射
	iounmap(s3c2440_i2c_regs);
}

module_init(i2c_bus_s3c2440_init);
module_exit(i2c_bus_s3c2440_exit);
MODULE_LICENSE("GPL");

