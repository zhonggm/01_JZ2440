
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#include <linux/fs.h>
#include <asm/uaccess.h>

static int major;
static struct class *class;
static struct i2c_client *at24cxx_client;


/*
传入			       : buf[0] : word_address
输出的数据存在: buf[0] : data_by_reading
*/
static ssize_t at24cxx_read (struct file *file, char __user *buf, size_t count, loff_t *off)
{
	unsigned char word_address, data_by_reading;
	unsigned long ret;
	
	ret = copy_from_user(&word_address, buf, 1);
	if(ret)
		printk("copy_from_user fail.\n");
	
	data_by_reading = i2c_smbus_read_byte_data(at24cxx_client, word_address);
	
	ret = copy_to_user(buf, &data_by_reading, 1);
	if(ret)
		printk("copy_to_user fail.\n");

	return 1;
}
/*
buf[0] : word_address
buf[1] : data_to_be_writed
*/
static ssize_t at24cxx_write (struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	unsigned char ker_buf[2];
	unsigned char word_address, data_to_be_writed;
	unsigned long ret;

	ret = copy_from_user(ker_buf, buf, 2);
	if(ret)
		printk("copy_from_user fail.\n");
	
	word_address = ker_buf[0];
	data_to_be_writed		 = ker_buf[1];

	if(!i2c_smbus_write_byte_data(at24cxx_client, word_address, data_to_be_writed))
		return 2;
	else
		return -EIO;
}


//定义一个file_operations 结构体
static struct file_operations at24cxx_fops = {
	.owner = THIS_MODULE,
	.read  = at24cxx_read,
	.write = at24cxx_write,
};


static const struct i2c_device_id at24cxx_id_table[] = {
	{ "at24c02", 0 },
	{}
};

//probe函数实现的目的就是创建/dev/at24cxx设备节点
static int __devinit at24cxx_probe(struct i2c_client *client,
		const struct i2c_device_id *dev_id)
{
//	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	at24cxx_client = client;

	major = register_chrdev(0, "at24cxx", &at24cxx_fops);
	class = class_create(THIS_MODULE, "at24cxx");
	device_create(class, NULL, MKDEV(major, 0), NULL, "at24cxx");

	return 0;
}

static int __devexit at24cxx_remove(struct i2c_client *client)
{
//	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(class, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "at24cxx");

	return 0;
}

static struct i2c_driver at24cxx_driver = {
	.driver	= {
		.name	= "shenzhen",
		.owner	= THIS_MODULE,
	},
	.probe		= at24cxx_probe,
	.remove		= __devexit_p(at24cxx_remove),
	.id_table	= at24cxx_id_table,
};

/*1. 分配/ 设置i2c_driver */
static int at24cxx_drv_init(void)
{
	/*2. 注册i2c_driver */
	i2c_add_driver(&at24cxx_driver);
	
	return 0;
}

static void at24cxx_drv_exit(void)
{
	/*2. 注销i2c_driver */
	i2c_del_driver(&at24cxx_driver);


}

module_init(at24cxx_drv_init);
module_exit(at24cxx_drv_exit);

MODULE_LICENSE("GPL");


