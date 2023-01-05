#ifndef __OLED_H__
#define __OLED_H__


extern void OLEDInit(void);
extern void OLEDPutChar(int page, int col, char c);
extern void OLEDPrint(int page, int col, char *str);


#endif

