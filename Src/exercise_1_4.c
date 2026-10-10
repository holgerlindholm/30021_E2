/*#include "gpio.h"

void uart_send_string(const char *str)
{
    while (*str)
    {
        // Wait until Transmit Data Register is empty (TXE)
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

        // Send current character
        USART_SendData(USART2, (uint8_t)*str++);
    }
}

// Time Structure
typedef struct {
    volatile uint8_t hours;
    volatile uint8_t minutes;
    volatile uint8_t seconds;
    volatile uint8_t hundredths;
} Time_t;

// Global Time Registers
volatile Time_t clock_time = {0, 0, 0, 0};
volatile uint8_t clock_running = 0;
volatile uint8_t second_changed_flag = 0;

// Split Time Storage
Time_t split1 = {0, 0, 0, 0};
Time_t split2 = {0, 0, 0, 0};

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

void initTimer2_100Hz(void)
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

void TIM2_IRQHandler(void)
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
}

void exercise_1_4(void)
{
    initJoystick();
    initLed();
    initTimer2_100Hz();

    __enable_irq();

    printf("\r\n--- Event-Driven Stopwatch Ready ---\r\n");
    printf("Controls: Center = Start/Stop | Left = Split 1 | Right = Split 2 | Down = Reset\r\n");

    while (1)
    {
        __WFI(); // Sleep until 100Hz Timer interrupt wakes the CPU

        // 1. Output time every time the second variable changes
        if (second_changed_flag)
        {
            second_changed_flag = 0;
            Time_t now = GetTimeAtomic(&clock_time);
            // printf("TIME: %02u:%02u:%02u\r\n", now.hours, now.minutes, now.seconds);
            char buffer[32];

            sprintf(buffer, "Time: %02u:%02u:%02u.%02u\r\n",
            		now.hours,
					now.minutes,
                    now.seconds,
                    now.hundredths);
            uart_send_string(buffer);
        }

        // 2. Process input event when joystick state changes
        if (joystick_flag)
        {
            joystick_flag = 0; // Clear flag
            uint8_t joystickState = current_joystick_state;

            // CENTER: Start / Stop
            if (joystickState & JOYSTICK_CENTER)
            {
                clock_running = !clock_running;
                Time_t now = GetTimeAtomic(&clock_time);
                if (clock_running)
                    printf("[STARTED] %02u:%02u:%02u.%02u\r\n", now.hours, now.minutes, now.seconds, now.hundredths);
                else
                    printf("[STOPPED] %02u:%02u:%02u.%02u\r\n", now.hours, now.minutes, now.seconds, now.hundredths);
            }
            // LEFT: Split 1
            else if (joystickState & JOYSTICK_LEFT)
            {
                split1 = GetTimeAtomic(&clock_time);
                printf("[SPLIT 1] %02u:%02u:%02u.%02u\r\n", split1.hours, split1.minutes, split1.seconds, split1.hundredths);
            }
            // RIGHT: Split 2
            else if (joystickState & JOYSTICK_RIGHT)
            {
                split2 = GetTimeAtomic(&clock_time);
                printf("[SPLIT 2] %02u:%02u:%02u.%02u\r\n", split2.hours, split2.minutes, split2.seconds, split2.hundredths);
            }
            // DOWN: Stop and Reset to 00:00:00.00
            else if (joystickState & JOYSTICK_DOWN)
            {
                clock_running = 0;

                __disable_irq();
                clock_time.hours = 0;
                clock_time.minutes = 0;
                clock_time.seconds = 0;
                clock_time.hundredths = 0;
                __enable_irq();

                printf("[RESET] 00:00:00.00\r\n");
            }
        }
    }
}
*/
