/* reference
 * drivers\hid\usbhid\usbmouse.c
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

static struct input_dev *uk_dev;

static char *usb_buf;//Ŀ��
static dma_addr_t usb_buf_phys;//�����ַ
static int len;//����
static struct urb *uk_urb;

static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },//�ӿ���ΪHID���ӿ�������BOOT��Э����MOUSE
	//{USB_DEVICE(0x1234,0x5678)},//������������������ֻ֧��VID = 0x1234,PID = 0x5678,�Ϳ����������һ��
	{ }	/* Terminating entry */
};

static void usbmouse_as_key_irq(struct urb *urb)
{
#if 0
	int i;
	static int cnt = 0;
	
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++){
		printk("%02x ", usb_buf[i]);
	}
	
	printk("\n");
#endif

	static unsigned char pre_val;

	/* USB������ݺ���
	 * data[0]: bit0-���, 1-����, 0-�ɿ�
	 *		    bit1-�Ҽ�, 1-����, 0-�ɿ�
	 *		    bit2-�м�, 1-����, 0-�ɿ� 
	 */
	if ((pre_val & (1<<0)) != (usb_buf[0] & (1<<0))){
	
		/* ��������˱仯 */
		input_event(uk_dev, EV_KEY, KEY_L, (usb_buf[0] & (1<<0)) ? 1 : 0);
		input_sync(uk_dev);
	}

	if ((pre_val & (1<<1)) != (usb_buf[0] & (1<<1))){
	
		/* �Ҽ������˱仯 */
		input_event(uk_dev, EV_KEY, KEY_S, (usb_buf[0] & (1<<1)) ? 1 : 0);
		input_sync(uk_dev);
	}

	if ((pre_val & (1<<2)) != (usb_buf[0] & (1<<2))){
	
		/* �м������˱仯 */
		input_event(uk_dev, EV_KEY, KEY_ENTER, (usb_buf[0] & (1<<2)) ? 1 : 0);
		input_sync(uk_dev);
	}
	
	pre_val = usb_buf[0];

	/* �����ύurb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
}


/*
intf��ʾ�ӿڣ�һ���ӿڱ�ʾһ���豸��
������¼�����������ֹ��ܣ��п��ܾͶ�Ӧ�����豸
*/
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int ret;

	//ͨ���ӿڵõ�USB dev�ṹ��
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *endpoint;
	struct usb_host_interface *interface;
	int pipe;

	#if 0
	printk("found usbmouse!\n");
	//��ӡ���豸��ţ�����ID����ƷID
	printk("bcdUSB = %x\n", dev->descriptor.bcdUSB);
	printk("VID    = 0x%x\n", dev->descriptor.idVendor);
	printk("PID    = 0x%x\n", dev->descriptor.idProduct);
	#endif

	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;

	/* a. ����һ��input_dev */
	uk_dev = input_allocate_device();
	
	/* b. ���� */
	/* b.1 �ܲ��������¼� */
	set_bit(EV_KEY, uk_dev->evbit);//������
	set_bit(EV_REP, uk_dev->evbit);//�ظ���
	
	/* b.2 �ܲ������������Щ�¼� */
	set_bit(KEY_L, 	   uk_dev->keybit);
	set_bit(KEY_S, 	   uk_dev->keybit);
	set_bit(KEY_ENTER, uk_dev->keybit);
	
	/* c. ע�� */
	ret = input_register_device(uk_dev);
	if(ret)
		printk("input_register_device fail.\n");
	
	/* d. Ӳ����ز��� */
	/* ���ݴ���3Ҫ��: Դ,Ŀ��,���� */
	/* Դ: USB�豸��ĳ���˵㣬���Դ�Ⱥ��жϱ�־λ���ֺ��豸�Ŷ˵�ţ��Լ����� */
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);

	/* ����: */
	len = endpoint->wMaxPacketSize;

	/* Ŀ��: */
	usb_buf = usb_alloc_coherent(dev, len, GFP_ATOMIC, &usb_buf_phys);

	/* ʹ��"3Ҫ��" */
	/* ����usb request block */
	uk_urb = usb_alloc_urb(0, GFP_KERNEL);
	/* ʹ��"3Ҫ������urb" */
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf, len,
					usbmouse_as_key_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma = usb_buf_phys;//USB�����õ����ݺ����ĳ���ڴ���д��д���ģ���Ҫ������Ҫд�������ַ�ˡ�
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;//���ñ�־

	/* ʹ��URB */
	usb_submit_urb(uk_urb, GFP_KERNEL);

	return 0;
}


static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
//	printk("disconnect usbmouse!\n");
	struct usb_device *dev = interface_to_usbdev(intf);

	usb_kill_urb(uk_urb);
	usb_free_urb(uk_urb);

	usb_free_coherent(dev, len, usb_buf, usb_buf_phys);
	input_unregister_device(uk_dev);
	input_free_device(uk_dev);
}

/* 1. ����/����usb_driver�ṹ��  */
static struct usb_driver usbmouse_as_key_driver = {
	.name		= "usbmouse_as_key_",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,
};

static int usbmouse_as_key_init(void)
{
	/* 2. ע�� */
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


