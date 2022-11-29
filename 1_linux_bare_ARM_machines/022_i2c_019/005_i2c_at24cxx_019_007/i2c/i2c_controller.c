#include "i2c_controller.h"
#include "../string_utils.h"


#define I2C_CONTROLLER_NUM 10

static p_i2c_controller p_i2c_controllers[I2C_CONTROLLER_NUM];
static p_i2c_controller p_i2c_controller_selected;


/*��һ��i2c_controller����������Ÿ��ֲ�ͬоƬ�Ĳ����ṹ��*/
void register_i2c_controller(p_i2c_controller p_i2c_con)
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


/* ����������ѡ��ĳ��I2C������ */
int select_i2c_controller(char *name)
{
	int i; 
	
	for(i = 0; i < I2C_CONTROLLER_NUM; i++)
		{
			if(p_i2c_controllers[i] && (!strcmp(name, p_i2c_controllers[i]->name)))
				{
					p_i2c_controller_selected = p_i2c_controllers[i];
					return 0;
				}
		}

	return -1;
}


/* ʵ��i2c_transfer�ӿں��� */
int i2c_transfer(p_i2c_msg msg, int num)
{
	return p_i2c_controller_selected->master_xfer(msg, num);
}

void i2c_init(void)
{
	/* ע�������i2c������ */
	s3c2440_i2c_con_addr();

	/* ѡ��ĳ��i2c������ */
	select_i2c_controller("s3c2440");

	/* ��������init���� */
	p_i2c_controller_selected->init();
	
}



