#include "gpio.h"

void initJoystick(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable clocks for GPIOA, GPIOB and GPIOC
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    // -------------------------
    // Configure PC0 and PC1
    // -------------------------

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

    GPIO_Init(GPIOC, &GPIO_InitStruct);


    // -------------------------
    // Configure PA4
    // -------------------------

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

    GPIO_Init(GPIOA, &GPIO_InitStruct);


    // -------------------------
    // Configure PB5 and PB0
    // -------------------------

    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;

    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint8_t readJoystick(void)
{
    uint8_t joystickState = 0;

    // Bit 0: Up
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4))
    {
        joystickState |= (1 << 0);
    }

    // Bit 1: Down
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0))
    {
        joystickState |= (1 << 1);
    }

    // Bit 2: Left
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1))
    {
        joystickState |= (1 << 2);
    }

    // Bit 3: Right
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0))
    {
        joystickState |= (1 << 3);
    }

    // Bit 4: Center
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5))
    {
        joystickState |= (1 << 4);
    }

    // Bits 5-7 remain 0

    return joystickState;
}

volatile uint8_t joystick_flag = 0;
volatile uint8_t current_joystick_state = 0;

static void HandleJoystickInterrupt(uint32_t line)
{
    if (EXTI_GetITStatus(line) != RESET)
    {
        // Capture full joystick bitmask state directly on edge change
        current_joystick_state = readJoystick();
        joystick_flag = 1;

        // Clear the pending interrupt bit
        EXTI_ClearITPendingBit(line);
    }
}

void EXTI0_IRQHandler(void)   { HandleJoystickInterrupt(EXTI_Line0); }
void EXTI1_IRQHandler(void)   { HandleJoystickInterrupt(EXTI_Line1); }
void EXTI4_IRQHandler(void)   { HandleJoystickInterrupt(EXTI_Line4); }
void EXTI9_5_IRQHandler(void) { HandleJoystickInterrupt(EXTI_Line5); }

void initEXTI9_5(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	// EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // Both rising and falling
	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}

void initEXTI(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Connect EXTI lines to GPIO pins */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0); // PC0 Right
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1); // PC1 Left
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); // PA4 Up
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5); // PB5 Center

    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    /* EXTI0: PC0 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_Init(&EXTI_InitStructure);

    /* EXTI1: PC1 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_Init(&EXTI_InitStructure);

    /* EXTI4: PA4 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_Init(&EXTI_InitStructure);

    /* EXTI5: PB5 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

    /* EXTI0 IRQ */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    /* EXTI1 IRQ */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    /* EXTI4 IRQ */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    /* EXTI9_5 IRQ (handles line 5) */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}


void initLed(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable GPIO clocks
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    // Red - PB4
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Green - PC7
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Blue - PA9
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Start with LED off
    setLed(LED_OFF);
}


void setLed(LedColor color)
{
    // Turn all colours OFF
    GPIO_SetBits(GPIOB, GPIO_Pin_4);   // Red OFF
    GPIO_SetBits(GPIOC, GPIO_Pin_7);   // Green OFF
    GPIO_SetBits(GPIOA, GPIO_Pin_9);   // Blue OFF

    // Turn requested colour(s) ON
    switch (color)
    {
        case LED_RED:
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            break;

        case LED_GREEN:
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
            break;

        case LED_BLUE:
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            break;

        case LED_YELLOW:
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
            break;

        case LED_CYAN:
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            break;

        case LED_MAGENTA:
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            break;

        case LED_WHITE:
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
            GPIO_ResetBits(GPIOA, GPIO_Pin_9);
            break;

        case LED_OFF:
        default:
            // All colours are already OFF
            break;
    }
}
