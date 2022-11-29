

/*
参考drivers\block\xd.c
参考drivers\block\z2ram.c
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

#define RAMBLOCK_SIZE (1024*1024)//设置容量是1M

static struct gendisk *ramblock_disk;
static request_queue_t *ramblock_queue;

static int major;//主设备号

static DEFINE_SPINLOCK(ramblock_lock);//自旋锁
static unsigned char *ramblock_buf;



static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	/* 容量 = heads * cylinders * sectors * 512，现在只有扇区数未知，可以被求出来 */
	geo->heads     = 2;
	geo->cylinders = 32;
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;
	
	return 0;
}

static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
	.getgeo = ramblock_getgeo,
};

//请求处理函数
static void do_ramblock_request(request_queue_t * q)
{
	//static int r_cnt = 0;
	//static int w_cnt = 0;
	struct request *req;
	
	//printk("do_ramblock_request %d\n", ++cnt);
	//以电梯调度算法从队列q中取出下一个请求
	while ((req = elv_next_request(q)) != NULL) {
		/*
		在这里操作硬件来完成想相应的读写工作。
		因为这里是用内存来模拟磁盘，所以我们直接用memcpy就可以完读写工作了。 
		*/
		/* 数据传输三要素: 源,目的,长度 */
		/* 源  / 目的: */
		unsigned long offset = req->sector * 512;

		/* 目的 / 源: */
		// req->buffer

		/* 长度: */		
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

//入口函数
static int ramblock_init(void)
{
	/*1. 分配一个gendisk结构体 */
	ramblock_disk = alloc_disk(16);/* 次设备号个数 : 分区个数 + 1，写为16就是说最多可以创建15个分区 */
	
	/* 2. 设置 */
	/* 2.1 分配/设置队列: 提供读写能力 */
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
	ramblock_disk->queue = ramblock_queue;
	
	/* 2.2 设置其他属性: 比如容量 */
	major = register_blkdev(0, "ramblock");  /* 用命令cat /proc/devices可以查看有哪些设备 */	
	ramblock_disk->major       = major;
	ramblock_disk->first_minor = 0;//第1个次设备号
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops        = &ramblock_fops;//这里是空的操作函数
	set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);

	/* 3. 硬件相关操作 */
	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);/* 分配一块内存 */
	
	/* 4. 注册 */
	add_disk(ramblock_disk);

	return 0;
}

//出口函数
static void ramblock_exit(void)
{
	unregister_blkdev(major, "ramblock");//注销
	del_gendisk(ramblock_disk);//删除
	put_disk(ramblock_disk);
	blk_cleanup_queue(ramblock_queue);//清除

	kfree(ramblock_buf);
}


module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("GPL");


