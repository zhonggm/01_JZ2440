#include "i2c_controller.h"

#define I2C_CONTROLLER_NUM 10

static p_i2c_controller p_i2c_controllers[I2C_CONTROLLER_NUM];
static p_i2c_controller p_i2c_controller_selected;




/*有一个i2c_controller数组用来存放各种不同芯片的操作结构体*/
void register_i2c_controller(p_i2c_controller *p_i2c_con)
{
	int i;

	for(i = 0; i < I2C_CONTROLLER_NUM; i++)
		{
			if(!p_i2c_controllers[i])
				{
					p_i2c_controllers[i] = p_i2c_con;
					return;
				}
		}
}


/* 根据名字来选择某款I2C控制器 */
int select_i2c_controller(char *name)
{
	int i; 
	
	for(i = 0; i < I2C_CONTROLLER_NUM; i++)
		{
			if(p_i2c_controllers[i] && (!strcmy(name, p_i2c_controllers[i]->name)))
				{
					p_i2c_controller_selected = p_i2c_controllers[i];
					return 0;
				}
		}

	return -1;
}


/* 实现i2c_transfer接口函数 */
int i2c_transfer(i2c_msg msg, int num)
{
	return p_i2c_controller_selected->master_xfer(msg, num);
}

void i2c_init(void)
{
	/* 注册下面的i2c控制器 */
	s3c2440_i2c_con_addr();

	/* 选择某款i2c控制器 */



	/* 调用它的init函数 */


	
}



