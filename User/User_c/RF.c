#include "RF.H"

u8 RF_READ_OK = 0;                  //解码成功标志
u8 RF_READ_ins = 0;                 //状态指示
u8 RF_READ_i = 0;                   //循环
u8 RF_READ_j = 0;                   //循环
u32 RF_READ_time = 0;               //计算时间
u8 RF_READ_data[RF_Rean_Len] = {0}; //数据
u8 RF_READ_Zj = 0;

// 读取，GPIO初始化
void RF_Read_GPIO_init(void)
{
    GPIO_InitTypeDef GPIO_Initstruct;             //声明GPIO初始化结构体
    RF_Read_GPIO_RCC;                             //打开GPIO时钟
    GPIO_Initstruct.GPIO_Mode = GPIO_Mode_IN;     //输入模式
    GPIO_Initstruct.GPIO_OType = GPIO_OType_OD;   //开漏输入模式
    GPIO_Initstruct.GPIO_Pin = RF_Read_GPIO_Pin;  //引脚0
    GPIO_Initstruct.GPIO_PuPd = GPIO_PuPd_UP;     //上拉模式
    GPIO_Initstruct.GPIO_Speed = GPIO_High_Speed; //高速模式
    GPIO_Init(RF_Read_GPIOx, &GPIO_Initstruct);   //初始化GPIO
}
//读取，外部中断初始化
void RF_Read_EXTI_init(void)
{
    EXTI_InitTypeDef EXTI_Initstruct;                      //创建外部中断初始化结构体
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); //打开时钟
    RF_Read_EXIT_Link;                                     //将GPIO与外部中断连接
    EXTI_Initstruct.EXTI_Line = RF_Read_EXIT_Pin;          //配置的是外部中断0
    EXTI_Initstruct.EXTI_LineCmd = ENABLE;                 //使能
    EXTI_Initstruct.EXTI_Mode = EXTI_Mode_Interrupt;       //选择中断模式
    EXTI_Initstruct.EXTI_Trigger = EXTI_Trigger_Rising;    //上升沿模式
    EXTI_Init(&EXTI_Initstruct);                           //初始化外部中断0
}
//读取，配置NVIC
void RF_Read_EXTI_NVIC(void)
{
    NVIC_InitTypeDef NVIC_Initstruct;                                            //声明NVIC初始化结构体
    NVIC_Initstruct.NVIC_IRQChannel = RF_Read_EXIT_IRQn;                         //配置的外部中断0
    NVIC_Initstruct.NVIC_IRQChannelCmd = ENABLE;                                 //使能
    NVIC_Initstruct.NVIC_IRQChannelPreemptionPriority = RF_Read_EXIT_Priority_1; //主优先级
    NVIC_Initstruct.NVIC_IRQChannelSubPriority = RF_Read_EXIT_Priority_2;        //副优先级
    NVIC_Init(&NVIC_Initstruct);                                                 //初始化外部中断0的NVIC
}
//读取，定时器初始化
void RF_Read_TIM_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_Init_Struct;              //声明定时器初始化结构体
    NVIC_InitTypeDef NVIC_Init_Struct;                    //声明NVIC初始化结构体
    RF_Read_TIM_RCC;                                      //打开时钟
    TIM_Init_Struct.TIM_ClockDivision = TIM_CKD_DIV1;     //滤波器不分频
    TIM_Init_Struct.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    //每次中断触发时间=[(TIM_Period+1)*(TIM_Prescaler+1)/(SystemCoreClock)] (s)
    TIM_Init_Struct.TIM_Prescaler = 84 - 1;
    TIM_Init_Struct.TIM_Period = 0xffff - 1;
    TIM_Init_Struct.TIM_RepetitionCounter = 0;             //高级定时器特有，这里写0就行
    TIM_TimeBaseInit(RF_Read_TIM_TIMx, &TIM_Init_Struct);  //调用函数初始
    TIM_ITConfig(RF_Read_TIM_TIMx, TIM_IT_Update, ENABLE); //启用溢出中断

    NVIC_Init_Struct.NVIC_IRQChannel = RF_Read_TIM_IRQn;                         //中断名称
    NVIC_Init_Struct.NVIC_IRQChannelCmd = ENABLE;                                //使能
    NVIC_Init_Struct.NVIC_IRQChannelPreemptionPriority = RF_Read_TIM_Priority_1; //主优先级1
    NVIC_Init_Struct.NVIC_IRQChannelSubPriority = RF_Read_TIM_Priority_2;        //副优先级1
    NVIC_Init(&NVIC_Init_Struct);                                                //初始化NVIC
    TIM_Cmd(RF_Read_TIM_TIMx, ENABLE);                                           //打开定时器
}
//读取初始化函数
void RF_Read_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //选定NVIC分组
    RF_Read_GPIO_init();
    RF_Read_EXTI_init();
    RF_Read_EXTI_NVIC();
    RF_Read_TIM_init();
}
// 发送初始化
void RF_Send_init(void)
{
    GPIO_InitTypeDef GPIO_Initstruct;             //声明GPIO初始化结构体
    RF_Send_GPIO_RCC;                             //打开GPIO时钟
    GPIO_Initstruct.GPIO_Mode = GPIO_Mode_OUT;    //输入模式
    GPIO_Initstruct.GPIO_OType = GPIO_OType_OD;   //开漏输入模式
    GPIO_Initstruct.GPIO_Pin = RF_Send_GPIO_Pin;  //引脚0
    GPIO_Initstruct.GPIO_PuPd = GPIO_PuPd_UP;     //上拉模式
    GPIO_Initstruct.GPIO_Speed = GPIO_High_Speed; //高速模式
    GPIO_Init(RF_Send_GPIOx, &GPIO_Initstruct);   //初始化GPIO
}
void RF_READ_decode(void)
{
    if (RF_READ_ins == 0) //初始化 检测到下降沿
    {
        TIM_SetCounter(RF_Read_TIM_TIMx, 0);
        RF_READ_ins = 1;
    }
    else if (RF_READ_ins == 1)
    {

        RF_READ_time = TIM_GetCounter(RF_Read_TIM_TIMx);             //计算从上次电平到此时间
        if (RF_READ_time > 8000 - 500 && RF_READ_time < 8000 + 2000) //数据码送入前的延迟
        {
            TIM_SetCounter(RF_Read_TIM_TIMx, 0);
            RF_READ_ins = 2; //开始解码
        }
        else
        {
            RF_READ_ins = 0; //复位
        }
        for (int i = 0; i < RF_Rean_Len; i++)
            RF_READ_data[i] = 0;
        if (RF_READ_ins == 2) //解码第一位码
        {
            Delay_us(600);
            if (GPIO_ReadInputDataBit(RF_Read_GPIOx, RF_Read_GPIO_Pin) == 1)
            {
                RF_READ_Zj = 1;
            }
            else if (GPIO_ReadInputDataBit(RF_Read_GPIOx, RF_Read_GPIO_Pin) == 0)
            {
                RF_READ_Zj = 0;
            }
            RF_READ_j = 0;
            RF_READ_i = 1;
        }
    }
    else if (RF_READ_ins == 2) //解码后面的
    {
        RF_READ_time = TIM_GetCounter(RF_Read_TIM_TIMx); //计算时间做验证
        TIM_SetCounter(RF_Read_TIM_TIMx, 0);
        if (RF_READ_time > 1050 - 500 && RF_READ_time < 1050 + 500) // 1.05ms左右
        {
            Delay_us(600);                                                   //延迟
            if (GPIO_ReadInputDataBit(RF_Read_GPIOx, RF_Read_GPIO_Pin) == 1) //判断这个时刻电平高低
            {                                                                //低
                RF_READ_Zj <<= 1;
                RF_READ_Zj |= 0x01;
            }
            else if (GPIO_ReadInputDataBit(RF_Read_GPIOx, RF_Read_GPIO_Pin) == 0)
            { //高
                RF_READ_Zj <<= 1;
                RF_READ_Zj &= 0xFE;
            }
            RF_READ_i++;
            if (RF_READ_i >= 8) // 8位数据写完 换行
            {
                RF_READ_i = 0;
                RF_READ_data[RF_READ_j] = RF_READ_Zj;
                RF_READ_Zj = 0;
                RF_READ_j++;
            }
        }
        else
        {
            RF_READ_ins = 0;
            RF_READ_Zj = 0;
        }
        if (RF_READ_j >= RF_Rean_Len) // 24位数据写完 完成标志写1 复位
        {
            RF_READ_j = 0;
            RF_READ_i = 0;
            RF_READ_OK = 1;
            RF_READ_ins = 0;
            RF_READ_Zj = 0;
        }
    }
}
void RF_WRITE_send_1(void) //发1
{
    GPIO_SetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);

    Delay_us(755);
    GPIO_ResetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_us(305);
}
void RF_WRITE_send_0(void) //发0
{
    GPIO_SetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_us(305);
    GPIO_ResetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_us(755);
}
void RF_WRITE_send_before(void) //发送诱导波后延迟
{
    RF_WRITE_send_1();
    RF_WRITE_send_0();
    RF_WRITE_send_1();
    GPIO_ResetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_ms(8);
}
void RF_WRITE_send_after(void) //发送结束波
{
    GPIO_SetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_us(305);
    GPIO_ResetBits(RF_Send_GPIOx, RF_Send_GPIO_Pin);
    Delay_ms(8);
}
void RF_Send(u8 *Dat, u8 Len) //发送1次
{
    u8 zj;
    RF_WRITE_send_before();
    for (int i = 0; i < Len; i++)
    {
        zj = Dat[i];
        for (int j = 0; j < 8; j++)
        {
            if (zj & 0x80)
            {
                RF_WRITE_send_1();
            }
            else
            {
                RF_WRITE_send_0();
            }
            zj <<= 1;
        }
    }
    RF_WRITE_send_after();
}

//外部中断0的中断服务函数
void RF_Read_EXTI_IRQHandler(void)
{
    if (EXTI_GetITStatus(RF_Read_EXIT_Pin) != RESET) //标志位被值位（产生中断）
    {
        RF_READ_decode();

        EXTI_ClearITPendingBit(RF_Read_EXIT_Pin); //清除中断标志位
    }
}
//定时器中断，不能没有
void RF_Read_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(RF_Read_TIM_TIMx, TIM_IT_Update) != RESET)
    {

        TIM_ClearITPendingBit(RF_Read_TIM_TIMx, TIM_IT_Update); //将中断标志清除
    }
}
