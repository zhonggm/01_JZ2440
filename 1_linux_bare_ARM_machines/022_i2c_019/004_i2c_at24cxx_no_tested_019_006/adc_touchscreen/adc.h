
#ifndef _ADC_H
#define _ADC_H

#define ENABLE_START_1 1
#define PRSCEN_1 1
#define AIN_0 0
#define PRSCVL 49

void adc_init(void);
int adc_read_ain0(void);

#endif /* _ADC_H */

