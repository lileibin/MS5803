#ifndef __MS5803_H__
#define __MS5803_H__

#include "math.h"
#include "string.h"
#include "stm32l4xx_hal.h"

/* Size of buffer */
#define BUFFERSIZE                       (COUNTOF(aTxBuffer) - 1)
#define SetCS()														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET)
#define ClrCS()														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET)

//************************CMD*********************************

#define	RESET		0x1E

#define	CD1_256		0x40		//OSR=256
#define	CD1_512		0x42		//OSR=512
#define	CD1_1024	0x44		//OSR=256
#define	CD1_2048	0x46		//OSR=256
#define	CD1_4096	0x48		//OSR=256

#define	CD2_256		0x50		//OSR=256
#define	CD2_512		0x52		//OSR=512
#define	CD2_1024	0x54		//OSR=256
#define	CD2_2048	0x56		//OSR=256
#define	CD2_4096	0x58		//OSR=256

#define ADC_Read	0x00

#define Cof1		0xA2
#define Cof2		0xA4
#define Cof3		0xA6
#define Cof4		0xA8
#define Cof5		0xAA
#define Cof6		0xAC
//#define CRC			0xAE

//***********************************************************

typedef volatile struct
{
	uint16_t C1;				//Pressure sensitivity
	
	uint16_t C2;				//Pressure offset
	
	uint16_t C3;				//Temperature coefficient of pressure sensitivity
	
	uint16_t C4;				//Temperature coefficient of pressure offset
	
	uint16_t C5;				//Reference temperature
	
	uint16_t C6;				//Temperature coefficient of the temperature
}PromVar;

/* Exported macro ------------------------------------------------------------*/
//#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

int SPI_Init(void);
void MS5803Init(void);
void StartCalculation(void);

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
