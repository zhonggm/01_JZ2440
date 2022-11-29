

/*
�ο�drivers\block\xd.c
�ο�drivers\block\z2ram.c
*/
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#define RAMBLOCK_SIZE (1024*1024)//����������1M

static struct gendisk *ramblock_disk;
static request_queue_t *ramblock_queue;

static int major;//���豸��

static DEFINE_SPINLOCK(ramblock_lock);//������
static unsigned char *ramblock_buf;



static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	/* ���� = heads * cylinders * sectors * 512������ֻ��������δ֪�����Ա������ */
	geo->heads     = 2;
	geo->cylinders = 32;
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;
	
	return 0;
}

static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
	.getgeo = ramblock_getgeo,
};

//��������
static void do_ramblock_request(request_queue_t * q)
{
	//static int r_cnt = 0;
	//static int w_cnt = 0;
	struct request *req;
	
	//printk("do_ramblock_request %d\n", ++cnt);
	//�Ե��ݵ����㷨�Ӷ���q��ȡ����һ������
	while ((req = elv_next_request(q)) != NULL) {
		/*
		���������Ӳ�����������Ӧ�Ķ�д������
		��Ϊ���������ڴ���ģ����̣���������ֱ����memcpy�Ϳ������д�����ˡ� 
		*/
		/* ���ݴ�����Ҫ��: Դ,Ŀ��,���� */
		/* Դ  / Ŀ��: */
		unsigned long offset = req->sector * 512;

		/* Ŀ�� / Դ: */
		// req->buffer

		/* ����: */		
		unsigned long len = req->current_nr_sectors * 512;

		if (rq_data_dir(req) == READ){
			//printk("do_ramblock_request read %d\n", ++r_cnt);
			memcpy(        req->buffer, ramblock_buf+offset, len);
		}else{
			//printk("do_ramblock_request write %d\n", ++w_cnt);
			memcpy(ramblock_buf+offset,         req->buffer, len);
		}		
		
		end_request(req, 1);/*wrap up, 0 = fail, 1 = success*/
	}
}

//��ں���
static int ramblock_init(void)
{
	/*1. ����һ��gendisk�ṹ�� */
	ramblock_disk = alloc_disk(16);/* ���豸�Ÿ��� : �������� + 1��дΪ16����˵�����Դ���15������ */
	
	/* 2. ���� */
	/* 2.1 ����/���ö���: �ṩ��д���� */
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
	ramblock_disk->queue = ramblock_queue;
	
	/* 2.2 ������������: �������� */
	major = register_blkdev(0, "ramblock");  /* ������cat /proc/devices���Բ鿴����Щ�豸 */	
	ramblock_disk->major       = major;
	ramblock_disk->first_minor = 0;//��1�����豸��
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops        = &ramblock_fops;//�����ǿյĲ�������
	set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);

	/* 3. Ӳ����ز��� */
	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);/* ����һ���ڴ� */
	
	/* 4. ע�� */
	add_disk(ramblock_disk);

	return 0;
}

//���ں���
static void ramblock_exit(void)
{
	unregister_blkdev(major, "ramblock");//ע��
	del_gendisk(ramblock_disk);//ɾ��
	put_disk(ramblock_disk);
	blk_cleanup_queue(ramblock_queue);//���

	kfree(ramblock_buf);
}


module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("GPL");


