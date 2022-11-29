
#ifndef _LCD_CONTROLLER_H
#define _LCD_CONTROLLER_H

#include "lcd.h"

/* LCD控制器结构体 */
typedef struct lcd_controller {
	char *name;
	void (*init)(p_lcd_params plcd);
	void (*enable)(void);/* 使能LCD控制器函数 */
	void (*disable)(void);/* 禁止LCD控制器函数 */
	
}lcd_controller, *p_lcd_controller;

#endif /* _LCD_CONTROLLER_H */



