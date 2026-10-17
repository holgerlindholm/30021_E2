#include "stm32f30x.h"
#include <stdio.h>
#include "timer.h"
#include "stm32f30x_tim.h"



// Split Time Storage

static Time_t GetTimeAtomic(volatile Time_t *src)
{
    Time_t temp;
    __disable_irq(); // Enter Critical Section
    temp.hours = src->hours;
    temp.minutes = src->minutes;
    temp.seconds = src->seconds;
    temp.hundredths = src->hundredths;
    __enable_irq();  // Exit Critical Section
    return temp;
}

void initTimer2_100Hz()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Prescaler = 6399;   // 64MHz / 6400 = 10kHz
    TIM_InitStructure.TIM_Period = 99;        // 10kHz / 100 = 100 Hz (10ms)
    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // Set Priority to 0 (Highest priority)
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM_Cmd(TIM2, ENABLE);
}



/*void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        // 1. Update Stopwatch Counter
        if (clock_running)
        {
            clock_time.hundredths++;
            if (clock_time.hundredths >= 100)
            {
                clock_time.hundredths = 0;
                clock_time.seconds++;
                second_changed_flag = 1;

                if (clock_time.seconds >= 60)
                {
                    clock_time.seconds = 0;
                    clock_time.minutes++;
                    if (clock_time.minutes >= 60)
                    {
                        clock_time.minutes = 0;
                        clock_time.hours++;
                        if (clock_time.hours >= 24)
                            clock_time.hours = 0;
                    }
                }
            }
        }

        // 2. Poll Joystick State every 10ms (100Hz Interrupt)
        static uint8_t last_state = 0;
        uint8_t state = readJoystick();

        // Trigger event flag only on state changes (Edge Detection + Debounce)
        if (state != last_state)
        {
            current_joystick_state = state;
            joystick_flag = 1; // Signal main event loop
            last_state = state;
        }

        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}*/

void TIM2_init_50Hz(){
    //configure TIM16
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);

    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Prescaler = 5018;
    TIM_InitStructure.TIM_Period = 255;

    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

}

void TIM16_init_50Hz(){
    //configure TIM16
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);

    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Prescaler = 5018;
    TIM_InitStructure.TIM_Period = 255;

    TIM_TimeBaseInit(TIM16, &TIM_InitStructure);

}


void TIM16_init_10kHz(){
    //configure TIM16
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_TimeBaseStructInit(&TIM_InitStructure);

    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Prescaler = 24;
    TIM_InitStructure.TIM_Period = 255;

    TIM_TimeBaseInit(TIM16, &TIM_InitStructure);

}

void TIM2_PWM_init()
{
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit(&TIM_OCInitStruct);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = 128;

    TIM_OC4Init(TIM2, &TIM_OCInitStruct);

    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIM2, ENABLE);

    TIM_Cmd(TIM2,ENABLE);
}

void TIM16_PWM_init()
{
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit(&TIM_OCInitStruct);

    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = 128;

    TIM_OC1Init(TIM16, &TIM_OCInitStruct);

    TIM_OC1PreloadConfig(TIM16, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIM16, ENABLE);

    TIM_Cmd(TIM16,ENABLE);
}


void GPIO_set_AF1_PA6(){
    //configure GPIO pins
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable clock for GPIOA */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, 6 ,GPIO_AF_1);
}

void GPIO_set_AF1_PB11(){
    //configure GPIO pins
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable clock for GPIOA */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, 11 ,GPIO_AF_1);
}

// Datasheet table 14: PA5, TIM2_CH1
void GPIO_set_AF_PA5(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable clock for GPIOA */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_1);  // GPIO_AF_1 :  OUT, TIM2, TIM15, TIM16, TIM17
}

//  22.2.4 TIM Driver: how to use it in Input Capture Mode
void TIM2_PWMInput_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_ICInitTypeDef       TIM_ICInitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure; // settings for one interrupt line in the NVIC (Nested Vectored Interrupt Controller)

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    GPIO_set_AF_PA5();

    // Input capture: CH1 rising edge, CH2 configured automatically
    // for the opposite edge on the same pin
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;

    // From page 520 //#define:  TIM_ICPSC_DIV1 ((uint16_t)0x0000)
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; // Capture performed each time an edge is detected on the capture input.
    TIM_ICInitStructure.TIM_ICFilter    = 0; // no filter, we can play around with this later
    TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

    // Reset the counter on every rising edge of TI1
    TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);
    TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);


    // Time base: 1 MHz tick, 32-bit counter
    TIM_TimeBaseStructInit(&TIM_InitStructure);
    TIM_InitStructure.TIM_Prescaler     = 71;
    TIM_InitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period        = 0xFFFFFFFF; // Counts up to max (overflow) Auto reload = period
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // no clock division
    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

    // Interrupt on CC1
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE); //  Enables or disables the specified TIM interrupts. (TIM Capture Compare 1 Interrupt source )
    TIM_Cmd(TIM2, ENABLE); // Enable the entire TIM counter
}

