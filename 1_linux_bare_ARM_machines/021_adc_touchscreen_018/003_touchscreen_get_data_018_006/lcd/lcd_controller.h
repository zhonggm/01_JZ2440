
#ifndef _LCD_CONTROLLER_H
#define _LCD_CONTROLLER_H

#include "lcd.h"

/* LCD�������ṹ�� */
typedef struct lcd_controller {
	char *name;
	void (*init)(p_lcd_params plcd);
	void (*enable)(void);/* ʹ��LCD���������� */
	void (*disable)(void);/* ��ֹLCD���������� */
	void (*init_palette)(void);/* LCD��������ɫ���ʼ�� */
	
}lcd_controller, *p_lcd_controller;

#endif /* _LCD_CONTROLLER_H */



