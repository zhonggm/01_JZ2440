
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>

//static struct i2c_board_info at24cxx_info[] __initdata = {
	//{ I2C_BOARD_INFO("at24c08", 0x1b), },
//};
static struct i2c_board_info at24cxx_info = { 
	I2C_BOARD_INFO("at24c08", 0x50), 
};


static struct i2c_client *at24cxx_client;
static const unsigned short addr_list[] = { 0x2c, 0x2d, I2C_CLIENT_END };


/*1. 分配/ 设置i2c_driver */
static int at24cxx_dev_init(void)
{
	struct i2c_adapter *i2c_adap;	
	struct i2c_board_info at24cxx_info;	

	memset(&at24cxx_info, 0, sizeof(struct i2c_board_info));	
	strlcpy(at24cxx_info.type, "at24c08", I2C_NAME_SIZE);
	
	//只有一个i2c设备，传0 就行。
	i2c_adap = i2c_get_adapter(0);

	/*2. 创建i2c_dev */
	at24cxx_client = i2c_new_probed_device(i2c_adap, &at24cxx_info, addr_list, NULL);
	i2c_put_adapter(i2c_adap);
		
	return 0;
}

static int at24cxx_dev_exit(void)
{
	/*2. 注销i2c_dev */
	i2c_del_driver(&at24cxx_driver);

	
	return 0;
}

module_init(at24cxx_drv_init);
module_exit(at24cxx_drv_exit);

MODULE_LICENSE("GPL");







