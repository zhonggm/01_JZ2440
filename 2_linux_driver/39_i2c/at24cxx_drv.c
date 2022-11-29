

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>


static const struct i2c_device_id at24cxx_id_table[] = {
	{ "at24c08", 0 },
	{}
};


static struct i2c_driver at24cxx_driver = {
	.driver	= {
		.name	= "shenzhen",
		.owner	= THIS_MODULE,
	},
	.probe		= at24cxx_probe,
	.remove		= __devexit_p(at24cxx_remove),
	.id_table	= at24cxx_id_table,
};

static int __devinit at24cxx_probe(struct i2c_client *,
		const struct i2c_device_id *)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


static int __devinit at24cxx_remove(struct i2c_client *)
{
	printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


/*1. ·ÖÅä/ ÉèÖÃi2c_driver */
static int at24cxx_drv_init(void)
{
	/*2. ×¢²ái2c_driver */
	i2c_add_driver(&at24cxx_driver);
	
	return 0;
}

static int at24cxx_drv_exit(void)
{
	/*2. ×¢Ïúi2c_driver */
	i2c_del_driver(&at24cxx_driver);

	
	return 0;
}

module_init(at24cxx_drv_init);
module_exit(at24cxx_drv_exit);

MODULE_LICENSE("GPL");


