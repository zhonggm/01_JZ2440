
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>


static struct i2c_client *at24cxx_client;
static const unsigned short addr_list[] = { 0x60, 0x50, I2C_CLIENT_END };


/*1. ����/ ����i2c_driver */
static int at24cxx_dev_init(void)
{
	struct i2c_adapter *i2c_adap;	
	struct i2c_board_info at24cxx_info;	

	memset(&at24cxx_info, 0, sizeof(struct i2c_board_info));	
	strlcpy(at24cxx_info.type, "at24c02", I2C_NAME_SIZE);
	
	//ֻ��һ��i2c�豸����0 ���С�
	i2c_adap = i2c_get_adapter(0);

	/*2. ����i2c_dev */
	at24cxx_client = i2c_new_probed_device(i2c_adap, &at24cxx_info, addr_list, NULL);
	i2c_put_adapter(i2c_adap);

	if(at24cxx_client)
		return 0;
	else 
		return -ENODEV;
}

static void at24cxx_dev_exit(void)
{
	/*2. ע��i2c_dev */
	i2c_unregister_device(at24cxx_client);


}

module_init(at24cxx_dev_init);
module_exit(at24cxx_dev_exit);

MODULE_LICENSE("GPL");







