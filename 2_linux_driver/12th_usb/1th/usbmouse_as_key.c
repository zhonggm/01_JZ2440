/* reference
 * drivers\hid\usbhid\usbmouse.c
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>



static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },//接口类为HID，接口子类是BOOT，协议是MOUSE
	//{USB_DEVICE(0x1234,0x5678)},//如果想让这个驱动程序只支持VID = 0x1234,PID = 0x5678,就可以添加上这一项
	{ }	/* Terminating entry */
};

/*
intf表示接口，一个接口表示一个设备，
声卡有录音、播放两种功能，有可能就对应两个设备
*/
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("found usbmouse!\n");
	return 0;
}

static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	printk("disconnect usbmouse!\n");
}

/* 1. 分配/设置usb_driver结构体  */
static struct usb_driver usbmouse_as_key_driver = {
	.name		= "usbmouse_as_key_",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,
};


static int usbmouse_as_key_init(void)
{
	/* 2. 注册 */
	usb_register(&usbmouse_as_key_driver);
	return 0;
}

static void usbmouse_as_key_exit(void)
{
	usb_deregister(&usbmouse_as_key_driver);	
}

module_init(usbmouse_as_key_init);
module_exit(usbmouse_as_key_exit);

MODULE_LICENSE("GPL");










