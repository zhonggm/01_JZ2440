
#ifndef _ADC_H
#define _ADC_H

#define ENABLE_START_1 1
#define PRSCEN_1 1
#define AIN_0 0//�����ѹ
#define AIN_1 1//���������ѹ
#define PRSCVL 49

extern void adc_init(int channel);
extern int adc_read(int channel);
 
#endif /* _ADC_H */

