#include "MS5803.h"

PromVar PROMData;
volatile int32_t CurrentTemp = 0,CurrentPress = 0;

/* SPI handler declaration */
	SPI_HandleTypeDef SpiHandle;
	
/* Buffer used for transmission */
uint8_t aTxBuffer[4];

uint8_t aRxBuffer[4];

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
	
	if(hspi->Instance == SPI1)
  {  		
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		
		__HAL_RCC_SPI1_CLK_ENABLE();

		//配置SPI1 I/O口PA5 PA6 PA7
		GPIO_InitStruct.Pin        = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull       = GPIO_NOPULL;
		GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate  = GPIO_AF5_SPI1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		//配置CS PB6		
		GPIO_InitStruct.Pin        = GPIO_PIN_6;
		GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull       = GPIO_PULLUP;
		GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
		SetCS();		
	}
}

int SPI_Init(void)
{	
  SpiHandle.Instance               = SPI1;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;


  SpiHandle.Init.Mode = SPI_MODE_MASTER;

  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
			return HAL_ERROR;
  }
	
	return HAL_OK;
}

//****************************************
//*功能：向MS5803写入命令
//*参数：CMD
//*返回：True/False

static int WriteCmd(uint8_t CMD)
{
	memset(aTxBuffer,0,sizeof(aTxBuffer));
	aTxBuffer[0] = CMD;
	
	ClrCS();
	
	if(HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, 1, 100))
	{
		SetCS();
		return 1;
	}
	else
	{
		SetCS();
		return 0;
	}
}

//****************************************
//*功能：根据指令从MS5803读相应数据
//*参数：CMD,Count
//*返回：True/False

static int ReadCmdData(uint8_t CMD,uint8_t Count)
{
	memset(aTxBuffer,0,sizeof(aTxBuffer));
	aTxBuffer[0] = CMD;
	
	ClrCS();

	if(HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*)aTxBuffer, (uint8_t *)aRxBuffer, Count+1, 100))
	{
		SetCS();
		return 1;
	}
	else
	{
		SetCS();
		return 0;
	}
	//memmove(aRxBuffer,aRxBuffer+1,Count);//
}

//****************************************
//*功能：复位MS5803
//*参数：
//*返回：

static void ResetDevice()
{
	WriteCmd(RESET);
	HAL_Delay(3);			//必须延时3ms
}

//****************************************
//*功能：获取PROM校准参数,只需要获取一次
//*参数：
//*返回：

static void GetPromData()
{
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof1,2);
	PROMData.C1 = (aRxBuffer[1]<<8) + aRxBuffer[2];
	
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof2,2);
	PROMData.C2 = (aRxBuffer[1]<<8) + aRxBuffer[2];
	
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof3,2);
	PROMData.C3 = (aRxBuffer[1]<<8) + aRxBuffer[2];
	
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof4,2);
	PROMData.C4 = (aRxBuffer[1]<<8) + aRxBuffer[2];
	
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof5,2);
	PROMData.C5 = (aRxBuffer[1]<<8) + aRxBuffer[2];
	
	memset(aRxBuffer,0,sizeof(aRxBuffer));
	ReadCmdData(Cof6,2);
	PROMData.C6 = (aRxBuffer[1]<<8) + aRxBuffer[2];
}

//****************************************
//*功能：MS5803初始化
//*参数：
//*返回：

void MS5803Init()
{
	//IIC初始化代码
	ResetDevice();
	GetPromData();
}

//****************************************
//*功能：获取温度和压力
//*参数：
//*返回：

void StartCalculation()
{
	uint32_t D1,D2;
	
	int32_t dT,TEMP;
	int64_t OFF,SENS;
	
	
	//转换压力
	WriteCmd(CD1_4096);
	HAL_Delay(10);			//必须延时10ms
	ReadCmdData(ADC_Read,3);
	D1 = (aRxBuffer[1]*pow(2,16)) + (aRxBuffer[2]*pow(2,8)) + aRxBuffer[3];
	
	//转换温度
	WriteCmd(CD2_4096);
	HAL_Delay(10);			//必须延时10ms
	ReadCmdData(ADC_Read,3);
	D2 = (aRxBuffer[1]*pow(2,16)) + (aRxBuffer[2]*pow(2,8)) + aRxBuffer[3];
	
	//计算温度
	dT = D2 - (PROMData.C5*pow(2,8));
	
	if(dT > 16777216)
	{
		dT = 16777216;
	}
	else if(dT < -16776960)
	{
		dT = -16776960;
	}
	
	TEMP = 2000 + ((((long long)dT)*PROMData.C6)/pow(2,23));
	
	//计算压力
	OFF = (PROMData.C2*pow(2,18)) + ((((long long)dT)*PROMData.C4)/pow(2,5));
	if(OFF > 51538821120)
	{
		OFF = 51538821120;
	}
	else if(OFF < -34358689800)
	{
		OFF = -34358689800;
	}
	
	//SENS = (((long long)PROMData.C1)*pow(2,17)) + ((((long long)dT)*PROMData.C3)/pow(2,7));//MS5803-05BA
	
	SENS = (((long long)PROMData.C1)*pow(2,17)) + ((((long long)dT)*PROMData.C3)/pow(2,6));
	
	if(SENS > 17179607040)
	{
		SENS = 17179607040;
	}
	else if(SENS < -8589672450)
	{
		SENS = -8589672450;
	}
	__nop();
	
	//Second order compensation
	long T2 = 0,OFF2 = 0,SENS2 = 0;
	
	if(TEMP < 2000)
	{
		T2 = (3*pow(dT,2))/pow(2,33);
		OFF2 = (3*pow((TEMP-2000),2))/pow(2,3);
		SENS2 = (7*pow((TEMP-2000),2))/pow(2,3);
		
		if(TEMP < -1500)
		{
			SENS2 += 3 * pow((TEMP+1500),2);
		}
	}
	
	CurrentTemp = TEMP - T2;
	OFF -= OFF2;
	SENS -= SENS2;
	
	CurrentPress = (((((long long)SENS)*D1)/pow(2,21)) - OFF)/pow(2,15);
	__nop();
}

