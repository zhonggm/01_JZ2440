
#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

extern void SPIFlashReadID(int *pMID, int *pDID);
extern void SPIFlashInit(void);
extern void SPIFlashEraseSector(unsigned int addr);
extern void SPIFlashProgram(unsigned int addr, unsigned char *buf, int len);
extern void SPIFlashRead(unsigned int addr, unsigned char *buf, int len);

#endif 


