#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[]      = { I2C_CLIENT_END };
/* 
��ֵַ��7λ 
��ʱ�豸��ַ��0x50��
��Ϊ0x60�Ļ������ڲ������豸��ַΪ0x60���豸������at24cxx_detect�������á�

*/
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END }; 

/* ����i2c_probe�е�&addr_data */
static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,  /* Ҫ����S�źź��豸��ַ���õ�ACK�ź�,����ȷ����������豸,���ܵ���detect���� */
	.probe		= ignore,/*ignore��ʡ�Ե���˼�����������Ǹ���i2c_probe����*/
	.ignore		= ignore,
	//.forces   /* ��forces����ĳ����ַ����ǿ����Ϊ����豸���� */
};

static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	printk("at24cxx_detect\n");//�豸��������
	return 0;
}

static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);
}

static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx_detach\n");
	return 0;
}

/* 1. ����һ��i2c_driver�ṹ�� */
/* 2. ����i2c_driver�ṹ�� */
static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name	= "at24cxx",
	},
//	.id = I2C_DRIVERID_DS1374,//����û���ã������Ȳ�д����������˵
	.attach_adapter = at24cxx_attach,//���
	.detach_client  = at24cxx_detach,//ж��
};

static int at24cxx_init(void)
{
	printk("at24cxx_init\n");
	i2c_add_driver(&at24cxx_driver);
	return 0;
}

static void at24cxx_exit(void)
{
	i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_init);
module_exit(at24cxx_exit);

MODULE_LICENSE("GPL");

