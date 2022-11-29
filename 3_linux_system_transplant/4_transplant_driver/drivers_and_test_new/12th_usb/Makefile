KERN_DIR = /home/book/workspace/3_sys_transplant/3_transplant_driver/linux-3.4.2

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= usbmouse_as_key.o
