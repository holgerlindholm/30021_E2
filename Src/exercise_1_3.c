#include "gpio.h"

void exercise_1_3(void)
{
    initJoystick();
    initLed();
    initEXTI();

    __enable_irq();

    printf("Joystick EXTI test started\r\n");

    uint8_t previousState = 0xFF;

    while (1)
    {
        __WFI();

        if (joystick_flag)
        {
            joystick_flag = 0;

            uint8_t joystickState = current_joystick_state;

            if (joystickState != previousState)
            {
                printf("Joystick state: 0x%02X\r\n", joystickState);

                if (joystickState & (1 << 0))
                {
                    setLed(LED_RED);
                    printf("UP\r\n");
                }
                else if (joystickState & (1 << 2))
                {
                    setLed(LED_BLUE);
                    printf("LEFT\r\n");
                }
                else if (joystickState & (1 << 3))
                {
                    setLed(LED_YELLOW);
                    printf("RIGHT\r\n");
                }
                else if (joystickState & (1 << 4))
                {
                    setLed(LED_WHITE);
                    printf("CENTER\r\n");
                }
                else
                {
                    setLed(LED_OFF);
                    printf("NONE\r\n");
                }

                previousState = joystickState;
            }
        }
    }
}
