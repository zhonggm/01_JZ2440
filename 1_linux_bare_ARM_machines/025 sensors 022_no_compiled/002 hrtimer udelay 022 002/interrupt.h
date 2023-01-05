

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

typedef void(*irq_func)(int);


extern void register_irq(int irq, irq_func fp);


#endif/* _INTERRUPT_H__ */





