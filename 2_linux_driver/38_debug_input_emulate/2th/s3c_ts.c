
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/plat-s3c24xx/ts.h>
#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>
#include <asm/uaccess.h>
#define LINE_DATA_SIZE 100

struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static struct input_dev *s3c_ts_dev;
static volatile struct s3c_ts_regs *s3c_ts_regs;
static struct timer_list ts_timer;
static struct timer_list replay_timer;


#define MYLOG_BUF_LEN (1024*1024)
#define INPUT_REPLAY   0
#define INPUT_TAG      1

//用于存放传进来的"回放数据"
static char *replay_buf;
static int replay_r = 0;
static int replay_w = 0;


static int major = 0;
/*
为了让mdev根据我们驱动给的信息自动创建设备节点，还得创建一个类，
在这个类下面创建一些设备
*/
static struct class *cls;
//声明自定的打印函数
extern int myprintk(const char *fmt, ...);

/* 把应用程序传入的数据写入replay_buf */
static ssize_t replay_write(struct file * file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	
	if (replay_w + size >= MYLOG_BUF_LEN){
		printk("replay_buf full!\n");
		return -EIO;
	}
	
	err = copy_from_user(replay_buf + replay_w, buf, size);
	if (err){
		return -EIO;
	}else{
		replay_w += size;
	}
	
	return size;
}


/* app: ioctl(fd, CMD, ..); */
static int replay_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned long ret;
	char buf[100];
	
	switch (cmd){
		case INPUT_REPLAY:
		{
			/* 启动回放: 根据replay_buf里的数据来上报事件 */
			replay_timer.expires = jiffies + 1;//这里初始时刻不能定为0
			printk("replay_ioctl add_timer\n");
			add_timer(&replay_timer);
			break;
		}
		case INPUT_TAG:
		{	/* 把输入的开始标志打印出来*/
			ret = copy_from_user(buf, (const void __user *)arg, 100);
			buf[99] = '\0';
			myprintk("%s\n", buf);
			break;
		}
	}
	return 0;
}

/*
功能:  读取一行数据
返回值为读取成功的字符个数: 
	        0 - 无数据，
	   非0 - 有数据 
*/
static int replay_get_line(char *line)
{
	int i = 0;
	
	/* 吃掉前导的空格、回车符 */
	while (replay_r <= replay_w){
		if ((replay_buf[replay_r] == ' ')  || \
			(replay_buf[replay_r] == '\n') || \
			(replay_buf[replay_r] == '\r') || \
			(replay_buf[replay_r] == '\t')){
			replay_r++;  /* 判断一行数据开始*/
		}else{
			break;
		}
	}
	
	while (replay_r <= replay_w){
		if ((replay_buf[replay_r] == '\n') || \
			(replay_buf[replay_r] == '\r')){
			break;/* 判断一行数据结束就退出*/
		}else{
			line[i] = replay_buf[replay_r];/* 读取一行数据，如果有空格也写入*/
			replay_r++;	
			i++;
			if(i == (LINE_DATA_SIZE - 1)){//只存LINE_DATA_SIZE个字符
				break;
			}
		}
	}

	line[i] = '\0';
	
	return i;	
}

//定时器超时处理函数
static void input_replay_timer_func(unsigned long data)
{
	/* 把replay_buf 里的一些数据取出来上报 
	 *  读出第1行数据, 确定time值, 上报第1行
	 *  继续读下1行数据, 如果它的time等于第1行的time, 上报
	 *					否则: mod_timer  
	 */
	unsigned int time;
	unsigned int type;
	unsigned int code;
	int val;

	static unsigned int pre_time = 0, pre_type = 0, pre_code = 0;
	static int pre_val = 0;
	
	char line[LINE_DATA_SIZE];
	int ret;

	if (pre_time != 0){
		/* 上报事件 */
		/* mod_timer 修改的时刻到达。*/
		input_event(s3c_ts_dev, pre_type, pre_code, pre_val);
	}

	while (1){
		
		ret = replay_get_line(line);
		
		if (ret == 0){
			//上报结束，退出。
			printk("end of input replay\n");
			del_timer(&replay_timer);
			pre_time = pre_type = pre_code = 0;
			pre_val = 0;
			replay_r = replay_w = 0;
			break;
		}

		/* 处理数据 */
		time = 0;
		type = 0;
		code = 0;
		val  = 0;
		sscanf(line, "%x %x %x %d", &time, &type, &code, &val);
		
		if (!time && !type && !code && !val){
			continue;
		}else{
			if ((pre_time == 0) || (time == pre_time)){
				/* 上报事件 */
				/*  
			  	pre_time == 0      : 上报第1行数据 
			  	time == pre_time : 上报相同时间的数据 
			  	*/
				input_event(s3c_ts_dev, type, code, val);
				if (pre_time == 0){
					pre_time = time;
				}
			}else{
				/* 
				根据下一个要上报的数据的时差，
				调用 mod_timer修改时刻。 
				*/
				mod_timer(&replay_timer, jiffies + (time - pre_time));				
				pre_time = time;
				pre_type = type;
				pre_code = code;
				pre_val  = val;
				break;
			}
		}
	}
}


//因为里面的写函数和控制函数已用到，所以放到这两个函数定义完后面。
static struct file_operations replay_fops = {
	.owner  = THIS_MODULE,
	.write = replay_write,
	.ioctl = replay_ioctl,
};


//进入按下“等待中断模式”
static void enter_wait_pen_down_mode(void)
{
	s3c_ts_regs->adctsc = 0xd3;
}

//进入松开“等待中断模式”
static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;//(Bit[8] = 1)
}

//进入自动转换XY模式
static void enter_measure_xy_mode(void)
{
	/*
	Bit[3]:Pull-up Switch Disable, 
	Bit[2]:Automatically sequencing conversion of X-Position and YPosition
	*/
	s3c_ts_regs->adctsc = (1<<3)|(1<<2);
}

//启动ADC
static void start_adc(void)
{
	//A/D conversion starts by enable
	s3c_ts_regs->adccon |= (1<<0);
}

//打印函数
void write_input_event_to_file(unsigned int time, unsigned int type, unsigned int code, int val)
{
	myprintk("0x%08x 0x%08x 0x%08x %d\n", time, type, code, val);
//	myprintk("0x%08x 0x%08x 0x%08x %d\n", time, EV_ABS, ABS_PRESSURE, 0);	
}

//软件过滤
static int s3c_filter_ts(int x[], int y[])
{
	#define ERR_LIMIT 10 //若定义了误差是 10.这是一个经验值。
	
	int avr_x, avr_y; //四 次测量值，两两的平均值。
	int det_x, det_y; //误差值。
	
	avr_x = (x[0] + x[1])/2; //取平均值
	avr_y = (y[0] + y[1])/2;
	det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]); //求出误差值。
	det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);
	if((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;  //若误差大过 ERR_TIMET 就认为错直接返回.
		
	avr_x = (x[1] + x[2])/2;
	avr_y = (y[1] + y[2])/2;
	det_x = (x[3] > avr_x) ? (x[3] - avr_x) : (avr_x - x[3]);
	det_y = (y[3] > avr_y) ? (y[3] - avr_y) : (avr_y - y[3]);
	if((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;
	
	return 1;
}

/*
write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);
write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);
write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
*/
//定时器中断处理函数
static void s3c_ts_timer_function(unsigned long data)
{
	if (s3c_ts_regs->adcdat0 & (1<<15)){
		
		/* 
		    已经松开，就等待被按下。
		    jiffies , type,  code, value
		*/
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);
//		printk("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
		
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);
				
		input_sync(s3c_ts_dev);
		write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);

		enter_wait_pen_down_mode();
		
	}else{
		/* 测量X/Y坐标 */
		enter_measure_xy_mode();
		start_adc();
	}
}


//TC中断处理函数
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	//2440 手册中 ADCDAT0 寄存器 bit15 置 0 时表示触摸笔按下状态，置 1 时表示触摸笔松开状态。
	if (s3c_ts_regs->adcdat0 & (1<<15)){
		//printk("pen up\n"); //已松开
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);

		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);

		input_sync(s3c_ts_dev);
		write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
		
		enter_wait_pen_down_mode();//进入等待按下检测
	}else{
		//printk("pen down\n");//已按下
		//enter_wait_pen_up_mode();  //进入等待松开检测
		enter_measure_xy_mode();
		start_adc();
	}
	
	return IRQ_HANDLED;
}


//ADC中断处理函数
static irqreturn_t adc_irq(int irq, void *dev_id)
{
	static int cnt = 0;
	int adcdat0, adcdat1;
	static int x[4], y[4]; //XY 坐标电压值都测量 4 次。
	
	/* 4.2,优化措施 2: 如果 ADC 完成时, 发现触摸笔已经松开, 则丢弃此次结果 */
		//ADCDAT0 寄存的低 10 位就是 X 坐标值.ADCDAT1 低 10 就是 Y 坐标值.
	adcdat0 = s3c_ts_regs->adcdat0;
	adcdat1 = s3c_ts_regs->adcdat1;
	
	if (s3c_ts_regs->adcdat0 & (1<<15)){ //ADCDAT0 bit15=1 时是松开状态
	
		/* 已经松开,就等待触摸笔按下模式。这时不打印. */
		cnt = 0;
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);
		
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);
		
		input_sync(s3c_ts_dev);
		write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
		
		enter_wait_pen_down_mode();
				
	}else{ //否则就是 ADCDAT0 bit15=0，是触摸笔按下的状态，这时打印 XY 坐标值。
	
		//ADCDAT0 寄存的低 10 位就是 X 坐标值.ADCDAT1 低 10 就是 Y 坐标值.打印完后再等待松开模式.
		//printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		/* 优化措施 3:  多次测量求平均值 */
		x[cnt] = adcdat0 & 0x3ff;
		y[cnt] = adcdat1 & 0x3ff;
		++cnt;
		if(cnt == 4){
			/* 优化措施4: 软件过滤 */
			if(s3c_filter_ts(x, y)){
				//printk("x = %d, y = %d \n", (x[0] + x[1] + x[2] + x[3]) / 4, (y[0] + y[1] + y[2] + y[3]) / 4);
				input_report_abs(s3c_ts_dev, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);/*X坐标*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);
				
				input_report_abs(s3c_ts_dev, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);/*Y坐标*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);
				
				input_report_abs(s3c_ts_dev, ABS_PRESSURE, 1);/*压力*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 1);
				
				input_report_key(s3c_ts_dev, BTN_TOUCH, 1);/*触摸按键*/
				write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 1);
				
				input_sync(s3c_ts_dev);/*同步事件*/
				write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
			}
			
			cnt = 0;
			enter_wait_pen_up_mode();//进入等待松开模式才能连续
			//到了4 次还是按着
			/* 启动定时器处理长按/滑动的情况 10ms ，HZ = 100 ，周期就是10ms */
			mod_timer(&ts_timer, jiffies + HZ/100);
			
		}else{
			//还没到达4次再进入测量
			enter_measure_xy_mode();
			start_adc();
		}
	}
	
	return IRQ_HANDLED;
}

static int s3c_ts_init(void)
{
	int ret;
	struct clk* clk;
	
	replay_buf = kmalloc(MYLOG_BUF_LEN, GFP_KERNEL);
	if (!replay_buf){
		printk("can't alloc for replay_buf\n");
		return -EIO;
	}
//	//1.分配一个 input_dev 结构体。 
	s3c_ts_dev = input_allocate_device();
	
//	//2.设置,设置分为两大类:
//	//2.1，能产生哪类事件; 事件位
	set_bit(EV_KEY, s3c_ts_dev->evbit);//能产生按键类事件
	set_bit(EV_ABS, s3c_ts_dev->evbit);//能产生触摸屏绝对位移事件	

//	//2.2, 能产生这类事件里的哪些事件. 按键位
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);//能产生按键类里的触摸屏事件
	input_set_abs_params(s3c_ts_dev, 		ABS_X, 0, 0x3FF, 0, 0);//X方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, 		ABS_Y, 0, 0x3FF, 0, 0);//Y方向最小值0，最大值0x3FF(10位ADC)
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 	  1, 0, 0);//压力方向最小值0，最大值1(要么按下，要么松开)
	
//	//3.注册设备驱动
	input_register_device(s3c_ts_dev);
	
//	//4.硬件相关的操作 
	/*4.1 使能时钟( CLKCON[15] )*/
	clk = clk_get(NULL, "adc");/* 参考s3c2410_ts.c里的probe函数 */
	clk_enable(clk);
	
	/*4.2 设置S3C2440的ADC / TS寄存器 */
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));

	/*
	设置 ADCCON 寄存器
	PRSCEN Bit[14]: A/D converter prescaler enable,1 = Enable
	PRSCVL Bit[13:6]: A/D converter prescaler value, set 49,
					A/D converter freq. = 50MHz/(49+1) = 1MHz
	ENABLE_START[0][0]:A/D conversion starts by enable,这里先设为0，以后要启动ADC时再设置。
	*/	
	s3c_ts_regs->adccon = (1<<14) | (49<<6);

	//注册中断处理函数
	ret = request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	ret = request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);

	/*
	优化错施1:
	设置ADCDLY为最大值，这使得电压稳定后再发出IRQ_TC中断
	*/
	s3c_ts_regs->adcdly = 0xffff;

	/* 优化措施5: 使用定时器处理长按,滑动的情况
	 * 
	 */
	init_timer(&ts_timer);
	ts_timer.function = s3c_ts_timer_function;//定时器中断处理函数
	add_timer(&ts_timer);//添加定时器

	enter_wait_pen_down_mode();//一开始先设置触摸笔等待被按下的模式

	//注册回放驱动
	major = register_chrdev(0, "input_replay", &replay_fops);

	cls = class_create(THIS_MODULE, "input_replay");
	device_create(cls, NULL, MKDEV(major, 0), "input_emu"); /* /dev/input_emu */

	//创建回放数据定时器
	init_timer(&replay_timer);
	replay_timer.function = input_replay_timer_func;

	return 0;
}

static void s3c_ts_exit(void)
{
	kfree(replay_buf);
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	
	//注销回放驱动
	unregister_chrdev(major, "input_replay");

	//释放 IRQ_TC.
	free_irq(IRQ_TC, NULL);
	
	//释放 IRQ_ADC 中断
	free_irq(IRQ_ADC, NULL);
	
	//iounmap 掉寄存器
	iounmap(s3c_ts_regs);
	
	//从内核释放此s3c_ts_dev 结构体
	input_unregister_device(s3c_ts_dev);
	
	//释放s3c_ts_dev 结构体空间.
	input_free_device(s3c_ts_dev);	
	del_timer(&ts_timer);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");//协议

