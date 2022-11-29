
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/proc_fs.h>

#define MYLOG_BUF_LEN (1024*1024)

struct proc_dir_entry *myentry;

char *mylog_buf;
char tmp_buf[1024];

static int mylog_r = 0;
static int mylog_r_for_read = 0;
static int mylog_w = 0;
static DECLARE_WAIT_QUEUE_HEAD(mymsg_waitq);

static int is_mylog_empty(void)
{
	return (mylog_r == mylog_w);
}
static int is_mylog_empty_for_read(void)
{
	return (mylog_r_for_read == mylog_w);
}

static int is_mylog_full(void)
{
	return ((mylog_w + 1)%MYLOG_BUF_LEN == mylog_r);
}

static void mylog_putc(char c)
{
	if (is_mylog_full())
	{
		/* 丢弃一个数据 */
		mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;
		
		//mylog_r_for_read 也要跟着往前挪
		if ((mylog_r_for_read + 1) % MYLOG_BUF_LEN == mylog_r)
		{
			mylog_r_for_read = mylog_r;
		}
	}
	//把数据放入buffer
	mylog_buf[mylog_w] = c;
	mylog_w = (mylog_w + 1) % MYLOG_BUF_LEN;

	/* 唤醒当前正在等待数据或者休眠的进程 */	
    wake_up_interruptible(&mymsg_waitq);   /* 唤醒休眠的进程 */	
}

static int mylog_getc(char *p)
{
	if (is_mylog_empty())
	{
		return 0;
	}
	
	*p = mylog_buf[mylog_r];
	mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;
	
	return 1;
}

static int mylog_getc_for_read(char *p)
{
	if (is_mylog_empty_for_read())
	{
		return 0;
	}
	
	*p = mylog_buf[mylog_r_for_read];
	mylog_r_for_read = (mylog_r_for_read + 1) % MYLOG_BUF_LEN;
	
	return 1;
}


int myprintk(const char *fmt, ...)
{
	va_list args;
	int i;
	int j;

	va_start(args, fmt);
	i = vsnprintf(tmp_buf, INT_MAX, fmt, args);
	va_end(args);
	
	for (j = 0; j < i; j++)
		mylog_putc(tmp_buf[j]);
		
	return i;
}

static ssize_t mymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	int error = 0;
	int i = 0;
	char c;

	/* 把mylog_buf的数据copy_to_user, return */
	if ((file->f_flags & O_NONBLOCK) && is_mylog_empty_for_read())
		return -EAGAIN;
	
	//如果mylog_buf 为空，进入休眠，等待有数据被唤醒
	error = wait_event_interruptible(mymsg_waitq, !is_mylog_empty_for_read());

	/* copy_to_user */
	while (!error && (mylog_getc_for_read(&c)) && i < count) {
		error = __put_user(c, buf);
		buf++;
		i++;
	}
	
	if (!error)
		error = i;
	
	return error;
}

static int mymsg_open (struct inode *inode, struct file *file)
{
	mylog_r_for_read = mylog_r;
	return 0;
}


const struct file_operations proc_mymsg_operations = {
	.open = mymsg_open,
	.read = mymsg_read,

};

static int mymsg_init(void)
{
	/* 分配1M内存空间 */
	mylog_buf = kmalloc(MYLOG_BUF_LEN, GFP_KERNEL);
	if(!mylog_buf){
		printk("can't alloc for mylog_buf\n");
		return -EIO;
	}

	myentry = create_proc_entry("mymsg", S_IRUSR, &proc_root);
	if (myentry)
		myentry->proc_fops = &proc_mymsg_operations;
	
	return 0;
}

static void mymsg_exit(void)
{
	remove_proc_entry("mymsg", &proc_root);
	/* 释放1M内存空间 */
	kfree(mylog_buf);
}

module_init(mymsg_init);
module_exit(mymsg_exit);
EXPORT_SYMBOL(myprintk);

MODULE_LICENSE("GPL");


