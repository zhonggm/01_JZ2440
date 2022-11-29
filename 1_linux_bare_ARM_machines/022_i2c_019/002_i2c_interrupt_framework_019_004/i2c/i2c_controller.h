
#ifndef _I2C_CONTROLLER_H__
#define _I2C_CONTROLLER_H__

typedef struct i2c_msg{
	unsigned int addr;/*7bits*/
	int flag;
	int len;
	int cnt_transferred;
	unsigned char *buf;
}i2c_msg, *p_i2c_msg;

typedef struct i2c_controller{
	int (*init)void;
	int (*master_xfer)(i2c_msg msg, int num);
	char *name;
}i2c_controller, *p_i2c_controller;



#endif /* _I2C_CONTROLLER_H__ */


