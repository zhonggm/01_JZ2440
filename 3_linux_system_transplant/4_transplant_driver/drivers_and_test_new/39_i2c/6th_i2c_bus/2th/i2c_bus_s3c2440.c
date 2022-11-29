#include <linux/kernel.h>
#include <linux/module.h>//THIS_MODULE��������

#include <linux/i2c.h>//I2C_M_RD����
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>//����irq_handler_t����

#include <linux/delay.h>//ndelay��������
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

#include <plat/gpio-cfg.h>//S3C2410_GPE14_IICSCL����
#include <mach/regs-gpio.h>


//#define PRINTK printk
#define PRINTK(...) 

//����i2c��״̬ö��
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

//�������ڱ������ݵĽṹ������s3c2440_i2c_xfer_data������
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
	s3c2440_i2c_xfer_data.state = STATE_START;//��λ״̬

	/* ������ */
	if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ 
		s3c2440_i2c_regs->iicds		 = s3c2440_i2c_xfer_data.msgs->addr << 1;
		s3c2440_i2c_regs->iicstat 	 = 0xb0;	// �������գ�����
	}else{ /* д���� */
		s3c2440_i2c_regs->iicds		 = s3c2440_i2c_xfer_data.msgs->addr << 1;
		s3c2440_i2c_regs->iicstat    = 0xf0; 	// �������ͣ�����
	}
}


static void s3c2440_i2c_stop(int err)
{
	s3c2440_i2c_xfer_data.state = STATE_STOP;
	s3c2440_i2c_xfer_data.err   = err;

	PRINTK("STATE_STOP, err = %d\n", err);

	/* ������ */
	if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ 
	
		// �������лָ�I2C����������P�ź�
		s3c2440_i2c_regs->iicstat = 0x90;
		s3c2440_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
		
	}else{ /* д���� */
		// �������������ָ�I2C����������P�ź�
		s3c2440_i2c_regs->iicstat = 0xd0;
		s3c2440_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
	}

	/* ����ֹͣ�ź�֮��Ҫ���� */
	wake_up(&s3c2440_i2c_xfer_data.wait);
}


static int s3c2440_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	unsigned long timeout;

	/* ��num��msg��I2C���ݷ��ͳ�ȥ/������ */
	s3c2440_i2c_xfer_data.msgs    = msgs;
	s3c2440_i2c_xfer_data.msn_num = num;
	s3c2440_i2c_xfer_data.cur_msg = 0;
	s3c2440_i2c_xfer_data.cur_ptr = 0;
	s3c2440_i2c_xfer_data.err     = -ENODEV;/*��ΪENODEV�����Ժ������ʱ�������err����Ϊ0��*/
	
	s3c2440_i2c_start();

	/* ���� */
	/* �˳�����������state Ϊstop  */
	timeout = wait_event_timeout(s3c2440_i2c_xfer_data.wait, (s3c2440_i2c_xfer_data.state == STATE_STOP), HZ * 5);	
	/* �ж��˳�ԭ�� */
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

/*  1. ���䡢����i2c_adapter  */
static struct i2c_adapter s3c2440_i2c_adapter = {
 .name			 = "s3c2440_100ask",
 .algo			 = &s3c2440_i2c_algo,
 .owner 		 = THIS_MODULE,
};

/* �����һ����Ϣ */
static int isLastMsg(void)
{
	return (s3c2440_i2c_xfer_data.cur_msg == s3c2440_i2c_xfer_data.msn_num - 1);
}

/* �Ѿ������һ������*/
static int isEndData(void)
{
	return (s3c2440_i2c_xfer_data.cur_ptr >= s3c2440_i2c_xfer_data.msgs->len);
}

/* ���������һ������*/
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
		case STATE_START : /* ����S���豸��ַ��,�����ж� */
		{
			PRINTK("Start\n");
			/* ���û���յ�ACK, ֹͣ�����ش��� */
			if (iicSt & S3C2410_IICSTAT_LASTBIT){
				s3c2440_i2c_stop(-ENODEV);
				break;
			}
			
			//��������һ����Ϣ���������һ�����ݣ���ô�ɹ���
			if (isLastMsg() && isEndData()){
				s3c2440_i2c_stop(0);
				break;
			}

			/* ����Ȳ���û��ack���ֲ������һ֡���ݣ�������һ��״̬ */
			if (s3c2440_i2c_xfer_data.msgs->flags & I2C_M_RD){ /* �� */
				//����Ƕ��Ļ�goto next_read
				s3c2440_i2c_xfer_data.state = STATE_READ;
				goto next_read;
			}else{											   /* д */
				/*�����д�Ļ��ͼ���������break��
				    �ͼ�����һ��case  STATE_WRITE ִ�з����ݲ�����
				   */
				s3c2440_i2c_xfer_data.state = STATE_WRITE;
			}	
		}
		case STATE_WRITE:
		{
			PRINTK("STATE_WRITE\n");
			/* ���û���յ�ACK, ֹͣ�����ش��� */
			if (iicSt & S3C2410_IICSTAT_LASTBIT){
				s3c2440_i2c_stop(-ENODEV);
				break;
			}
			/* �����ǰmsg��������Ҫ���� */
			if (!isEndData()){  
				s3c2440_i2c_regs->iicds = s3c2440_i2c_xfer_data.msgs->buf[s3c2440_i2c_xfer_data.cur_ptr];
				s3c2440_i2c_xfer_data.cur_ptr++;
				
				// ������д��IICDS����Ҫһ��ʱ����ܳ�����SDA����
				ndelay(50);	
				
				s3c2440_i2c_regs->iiccon = 0xaf;// �ָ�I2C����
				break;				
			}else if (!isLastMsg()){
				/* ��ʼ������һ����Ϣ */
				s3c2440_i2c_xfer_data.msgs++;
				s3c2440_i2c_xfer_data.cur_msg++;
				s3c2440_i2c_xfer_data.cur_ptr = 0;
				s3c2440_i2c_xfer_data.state = STATE_START;
				/* ����START�źźͷ����豸��ַ */
				s3c2440_i2c_start();
				break;
			}else{
				/* �����һ����Ϣ�����һ�����ݾ�ֹͣ�������� */
				s3c2440_i2c_stop(0);
				break;				
			}
			break;
		}
		case STATE_READ:
		{
			PRINTK("STATE_READ\n");
			/* 
			�������� 
			��next_read֮ǰҪ�Ȱ����ݶ�������һ���������ж�ֱ�ӽ���
			STATE_READ�Ļ��ͱ����Ѿ�������һ�����ݡ�
			*/
			s3c2440_i2c_xfer_data.msgs->buf[s3c2440_i2c_xfer_data.cur_ptr] = s3c2440_i2c_regs->iicds;			
			s3c2440_i2c_xfer_data.cur_ptr++;
next_read:
			if (!isEndData()){ /* �������û����,  ������������� */
				if (isLastData()){  /* ��������������������һ��, ����ack */
					s3c2440_i2c_regs->iiccon = 0x2f;// �ָ�I2C���䣬���յ���һ����ʱ��ACK
				}else{
					s3c2440_i2c_regs->iiccon = 0xaf;// �ָ�I2C���䣬���յ���һ����ʱ����ACK
				}				
				break;
			}else if (!isLastMsg()){
				/* ��ʼ������һ����Ϣ */
				s3c2440_i2c_xfer_data.msgs++;
				s3c2440_i2c_xfer_data.cur_msg++;
				s3c2440_i2c_xfer_data.cur_ptr = 0;
				s3c2440_i2c_xfer_data.state = STATE_START;
				/* ����START�źźͷ����豸��ַ */
				s3c2440_i2c_start();
				break;
			}else{
				/* �����һ����Ϣ�����һ������ */
				s3c2440_i2c_stop(0);
				break;								
			}
			break;
		}
		default: break;
	}
	/* ��i2c�������жϱ�־ */
	s3c2440_i2c_regs->iiccon &= ~(S3C2410_IICCON_IRQPEND);

	return IRQ_HANDLED;	
}


static void s3c2440_i2c_init(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "i2c");//�ҵ�ʱ��
	clk_enable(clk);           //ʹ��ʱ��
	
	// ѡ�����Ź��ܣ�GPE15:IICSDA, GPE14:IICSCL
	s3c_gpio_cfgpin(S3C2410_GPE(14), S3C2410_GPE14_IICSCL);
	s3c_gpio_cfgpin(S3C2410_GPE(15), S3C2410_GPE15_IICSDA);

   /* bit[7]  = 1, ʹ��ACK
	 * bit[6] = 0, IICCLK = PCLK/16
	 * bit[5] = 1, ʹ���ж�
	 * bit[3:0] = 0xf, Tx clock = IICCLK/16
	 * PCLK = 50MHz, IICCLK = 3.125MHz, Tx Clock = 0.195MHz
	 */
	s3c2440_i2c_regs->iiccon = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf
	s3c2440_i2c_regs->iicadd  = 0x10;	  // S3C24xx slave address = [7:1]
	s3c2440_i2c_regs->iicstat = 0x10;	  //ʹ�� I2C�������(Rx/Tx)
}

static int i2c_bus_s3c2440_init(void)
{
	/* 2. Ӳ����ص����� *///�������ַ�������ַ��ӳ��
	s3c2440_i2c_regs = ioremap(0x54000000, sizeof(struct s3c2440_i2c_regs));

	s3c2440_i2c_init();

	/*ע���ж�*/
	request_irq(IRQ_IIC, s3c2440_i2c_xfer_irq, 0, "s3c2440-i2c", NULL);

	/*��ʼ���ȴ�����*/
	init_waitqueue_head(&s3c2440_i2c_xfer_data.wait);
		
	/* 3. ע�������� i2c_adapter  */	
	i2c_add_adapter(&s3c2440_i2c_adapter);
	
	return 0;
}

static void i2c_bus_s3c2440_exit(void)
{
	/* 3. ע��������i2c_adapter  */	
	i2c_del_adapter(&s3c2440_i2c_adapter);
	free_irq(IRQ_IIC, NULL);
	//�����ַӳ��
	iounmap(s3c2440_i2c_regs);
}

module_init(i2c_bus_s3c2440_init);
module_exit(i2c_bus_s3c2440_exit);
MODULE_LICENSE("GPL");

