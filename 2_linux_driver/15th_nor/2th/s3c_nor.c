/*
参考 linux-2.6.22.6\drivers\mtd\maps\Physmap.c

*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>

struct map_info *s3c_nor_map;
struct mtd_info	*s3c_nor_mtd;

static struct mtd_partition s3c_nor_parts[] = { 
	[0] = { 
		.name = "bootloader_nor", //分区名字 
		.size = 0x00040000, //分区大小 
		.offset = 0, //分区偏移值 
	}, 
	[1] = { 
		.name = "root_nor", 
		.offset = MTDPART_OFS_APPEND, 
		.size = MTDPART_SIZ_FULL, //剩下的所有空间大小 
	} 
};

//入口函数
static int s3c_nor_init(void)
{

	/*1. 分配map_info结构体*/
	s3c_nor_map = kzalloc(sizeof(struct map_info), GFP_KERNEL);
	
	/*2. 设置 : 物理基地址(phys)，大小(size)，位宽(bankwidth)，虚拟地址(virt)*/
	s3c_nor_map->name = "s3c_nor";
	s3c_nor_map->phys = 0;
	s3c_nor_map->size = 0x1000000;/* 这个的大小一定要大于等于nor的真正大小，这里设为1M */
	s3c_nor_map->bankwidth = 2;/* 2 * 8 = 16 bit*/
	s3c_nor_map->virt = ioremap(s3c_nor_map->phys, s3c_nor_map->size);

	simple_map_init(s3c_nor_map);//简单地初始化

	/*3. 使用 : 调用nor flash协议层提供的函数来识别 */
	printk("use cfi_probe\n");
	s3c_nor_mtd = do_map_probe("cfi_probe", s3c_nor_map);
	
	//如果没有识别出来就用老的
	if (!s3c_nor_mtd)
	{
		printk("use jedec_probe\n");
		s3c_nor_mtd = do_map_probe("jedec_probe", s3c_nor_map);
	}
	
	//如果还没有识别出来老的，就返回。	
	if (!s3c_nor_mtd)
	{		
		iounmap(s3c_nor_map->virt);
		kfree(s3c_nor_map);
		return -EIO;
	}

	/*4. add_mtd_partitions */
	add_mtd_partitions(s3c_nor_mtd, s3c_nor_parts, 2);

	
	return 0;
}

//出口函数
static void s3c_nor_exit(void)
{
	del_mtd_partitions(s3c_nor_mtd);
	iounmap(s3c_nor_map->virt);
	kfree(s3c_nor_map);
}

module_init(s3c_nor_init);
module_exit(s3c_nor_exit);
MODULE_LICENSE("GPL");


