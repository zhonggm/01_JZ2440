
#ifndef _TIMER_H_
#define _TIMER_H_

typedef void(*timer_func)(void);

typedef struct timer_desc{
	char *name;
	timer_func fp;
}timer_desc, *p_timer_func;

extern int register_timer(char *name, timer_func fp);
extern void unregister_timer(char *name);
extern void mdelay(int m);
extern void hrtimer_test(void);



#endif/* _TIMER_H_*/

