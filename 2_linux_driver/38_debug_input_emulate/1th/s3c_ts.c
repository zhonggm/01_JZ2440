
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

//�����Զ��Ĵ�ӡ����
extern int myprintk(const char *fmt, ...);

//���밴�¡��ȴ��ж�ģʽ��
static void enter_wait_pen_down_mode(void)
{
	s3c_ts_regs->adctsc = 0xd3;
}

//�����ɿ����ȴ��ж�ģʽ��
static void enter_wait_pen_up_mode(void)
{
	s3c_ts_regs->adctsc = 0x1d3;//(Bit[8] = 1)
}

//�����Զ�ת��XYģʽ
static void enter_measure_xy_mode(void)
{
	/*
	Bit[3]:Pull-up Switch Disable, 
	Bit[2]:Automatically sequencing conversion of X-Position and YPosition
	*/
	s3c_ts_regs->adctsc = (1<<3)|(1<<2);
}

//����ADC
static void start_adc(void)
{
	//A/D conversion starts by enable
	s3c_ts_regs->adccon |= (1<<0);
}

//��ӡ����
void write_input_event_to_file(unsigned int time, unsigned int type, unsigned int code, int val)
{
	myprintk("0x%08x 0x%08x 0x%08x %d\n", time, type, code, val);
//	myprintk("0x%08x 0x%08x 0x%08x %d\n", time, EV_ABS, ABS_PRESSURE, 0);	
}

//�������
static int s3c_filter_ts(int x[], int y[])
{
	#define ERR_LIMIT 10 //������������� 10.����һ������ֵ��
	
	int avr_x, avr_y; //�� �β���ֵ��������ƽ��ֵ��
	int det_x, det_y; //���ֵ��
	
	avr_x = (x[0] + x[1])/2; //ȡƽ��ֵ
	avr_y = (y[0] + y[1])/2;
	det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]); //������ֵ��
	det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);
	if((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;  //������� ERR_TIMET ����Ϊ��ֱ�ӷ���.
		
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
//��ʱ���жϴ�����
static void s3c_ts_timer_function(unsigned long data)
{
	if (s3c_ts_regs->adcdat0 & (1<<15)){
		
		/* 
		    �Ѿ��ɿ����͵ȴ������¡�
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
		/* ����X/Y���� */
		enter_measure_xy_mode();
		start_adc();
	}
}


//TC�жϴ�����
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	//2440 �ֲ��� ADCDAT0 �Ĵ��� bit15 �� 0 ʱ��ʾ�����ʰ���״̬���� 1 ʱ��ʾ�������ɿ�״̬��
	if (s3c_ts_regs->adcdat0 & (1<<15)){
		//printk("pen up\n"); //���ɿ�
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);

		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);

		input_sync(s3c_ts_dev);
		write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
		
		enter_wait_pen_down_mode();//����ȴ����¼��
	}else{
		//printk("pen down\n");//�Ѱ���
		//enter_wait_pen_up_mode();  //����ȴ��ɿ����
		enter_measure_xy_mode();
		start_adc();
	}
	
	return IRQ_HANDLED;
}


//ADC�жϴ�����
static irqreturn_t adc_irq(int irq, void *dev_id)
{
	static int cnt = 0;
	int adcdat0, adcdat1;
	static int x[4], y[4]; //XY �����ѹֵ������ 4 �Ρ�
	
	/* 4.2,�Ż���ʩ 2: ��� ADC ���ʱ, ���ִ������Ѿ��ɿ�, �����˴ν�� */
		//ADCDAT0 �Ĵ�ĵ� 10 λ���� X ����ֵ.ADCDAT1 �� 10 ���� Y ����ֵ.
	adcdat0 = s3c_ts_regs->adcdat0;
	adcdat1 = s3c_ts_regs->adcdat1;
	
	if (s3c_ts_regs->adcdat0 & (1<<15)){ //ADCDAT0 bit15=1 ʱ���ɿ�״̬
	
		/* �Ѿ��ɿ�,�͵ȴ������ʰ���ģʽ����ʱ����ӡ. */
		cnt = 0;
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 0);
		
		input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 0);
		
		input_sync(s3c_ts_dev);
		write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
		
		enter_wait_pen_down_mode();
				
	}else{ //������� ADCDAT0 bit15=0���Ǵ����ʰ��µ�״̬����ʱ��ӡ XY ����ֵ��
	
		//ADCDAT0 �Ĵ�ĵ� 10 λ���� X ����ֵ.ADCDAT1 �� 10 ���� Y ����ֵ.��ӡ����ٵȴ��ɿ�ģʽ.
		//printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		/* �Ż���ʩ 3:  ��β�����ƽ��ֵ */
		x[cnt] = adcdat0 & 0x3ff;
		y[cnt] = adcdat1 & 0x3ff;
		++cnt;
		if(cnt == 4){
			/* �Ż���ʩ4: ������� */
			if(s3c_filter_ts(x, y)){
				//printk("x = %d, y = %d \n", (x[0] + x[1] + x[2] + x[3]) / 4, (y[0] + y[1] + y[2] + y[3]) / 4);
				input_report_abs(s3c_ts_dev, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);/*X����*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);
				
				input_report_abs(s3c_ts_dev, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);/*Y����*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);
				
				input_report_abs(s3c_ts_dev, ABS_PRESSURE, 1);/*ѹ��*/
				write_input_event_to_file(jiffies, EV_ABS, ABS_PRESSURE, 1);
				
				input_report_key(s3c_ts_dev, BTN_TOUCH, 1);/*��������*/
				write_input_event_to_file(jiffies, EV_KEY, BTN_TOUCH, 1);
				
				input_sync(s3c_ts_dev);/*ͬ���¼�*/
				write_input_event_to_file(jiffies, EV_SYN, SYN_REPORT, 0);
			}
			
			cnt = 0;
			enter_wait_pen_up_mode();//����ȴ��ɿ�ģʽ��������
			//����4�λ��ǰ���
			/* ������ʱ��������/��������� 10ms ��HZ = 100 �����ھ���10ms */
			mod_timer(&ts_timer, jiffies + HZ/100);
			
		}else{
			//��û����4���ٽ������
			enter_measure_xy_mode();
			start_adc();
		}
	}
	
	return IRQ_HANDLED;
}

static int s3c_ts_init(void)
{
	struct clk* clk;

//	//1.����һ�� input_dev �ṹ�塣 
	s3c_ts_dev = input_allocate_device();
	
//	//2.����,���÷�Ϊ������:
//	//2.1���ܲ��������¼�; �¼�λ
	set_bit(EV_KEY, s3c_ts_dev->evbit);//�ܲ����������¼�
	set_bit(EV_ABS, s3c_ts_dev->evbit);//�ܲ�������������λ���¼�	

//	//2.2, �ܲ��������¼������Щ�¼�. ����λ
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);//�ܲ�����������Ĵ������¼�
	input_set_abs_params(s3c_ts_dev, 		ABS_X, 0, 0x3FF, 0, 0);//X������Сֵ0�����ֵ0x3FF(10λADC)
	input_set_abs_params(s3c_ts_dev, 		ABS_Y, 0, 0x3FF, 0, 0);//Y������Сֵ0�����ֵ0x3FF(10λADC)
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 	  1, 0, 0);//ѹ��������Сֵ0�����ֵ1(Ҫô���£�Ҫô�ɿ�)
	
//	//3.ע���豸����
	input_register_device(s3c_ts_dev);
	
//	//4.Ӳ����صĲ��� 
	/*4.1 ʹ��ʱ��( CLKCON[15] )*/
	clk = clk_get(NULL, "adc");/* �ο�s3c2410_ts.c���probe���� */
	clk_enable(clk);
	
	/*4.2 ����S3C2440��ADC / TS�Ĵ��� */
	s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));

	/*
	���� ADCCON �Ĵ���
	PRSCEN Bit[14]: A/D converter prescaler enable,1 = Enable
	PRSCVL Bit[13:6]: A/D converter prescaler value, set 49,
					A/D converter freq. = 50MHz/(49+1) = 1MHz
	ENABLE_START[0][0]:A/D conversion starts by enable,��������Ϊ0���Ժ�Ҫ����ADCʱ�����á�
	*/	
	s3c_ts_regs->adccon = (1<<14) | (49<<6);

	//ע���жϴ�����
	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);

	/*
	�Ż���ʩ1:
	����ADCDLYΪ���ֵ����ʹ�õ�ѹ�ȶ����ٷ���IRQ_TC�ж�
	*/
	s3c_ts_regs->adcdly = 0xffff;

	/* �Ż���ʩ5: ʹ�ö�ʱ��������,���������
	 * 
	 */
	init_timer(&ts_timer);
	ts_timer.function = s3c_ts_timer_function;//��ʱ���жϴ�����
	add_timer(&ts_timer);//��Ӷ�ʱ��

	enter_wait_pen_down_mode();//һ��ʼ�����ô����ʵȴ������µ�ģʽ

	return 0;
}

static void s3c_ts_exit(void)
{
	//�ͷ� IRQ_TC.
	free_irq(IRQ_TC, NULL);
	
	//�ͷ� IRQ_ADC �ж�
	free_irq(IRQ_ADC, NULL);
	
	//iounmap ���Ĵ���
	iounmap(s3c_ts_regs);
	
	//���ں��ͷŴ�s3c_ts_dev �ṹ��
	input_unregister_device(s3c_ts_dev);
	
	//�ͷ�s3c_ts_dev �ṹ��ռ�.
	input_free_device(s3c_ts_dev);	
	del_timer(&ts_timer);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");//Э��

