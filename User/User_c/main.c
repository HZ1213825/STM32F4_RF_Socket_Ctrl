#include "stm32f4xx.h" // 注意更改这个文件的#define STM32F401xx为自己的芯片型号
#include "Delay.h"
#include "Usart.h"
#include "RF_433M.H"
int main()
{
	u8 DAT[3] = {0xA5, 0x6D, 0x12};
	Usart_init();
	// RF_Read_Init();
	RF_Send_init();
	while (1)
	{
		RF_Send(DAT, 3);
		// if (RF_READ_OK)
		// {
		// 	for (int i = 0; i < 3; i++)
		// 	{
		// 		printf("%2X", RF_READ_data[i]);
		// 	}
		// 	printf("\r\n");
		// 	RF_READ_OK = 0;
		// }
	}
}
