#include "timer.h"
#include "stm32f30x.h"
#include "gpio.h"

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

#define TIM_TICK_US  1.0f    // (PSC + 1) / 72 MHz = 1 micro second

volatile uint32_t ICValue1, ICValue2; // Initialize variables to store last readings of the CCR1 and CCR2
volatile uint8_t  ICValid = 0; //Flag to see if new reading is available

// ISR = Interrupt Service Routine.
void TIM2_IRQHandler(void) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

    ICValue1 = TIM_GetCapture1(TIM2); // Period from CCR1
    ICValue2 = TIM_GetCapture2(TIM2); // Width from CCR2
    ICValid = 1;
}

#define AVG_N 10

static uint32_t pbuf[AVG_N], hbuf[AVG_N];
static uint64_t psum = 0, hsum = 0;
static uint8_t  avg_idx = 0, avg_count = 0;

// Add one measurement (period and high time, in ticks)
void avg_add(uint32_t p, uint32_t h)
{
    psum -= pbuf[avg_idx];          // remove the oldest sample
    hsum -= hbuf[avg_idx];
    pbuf[avg_idx] = p;              // store the new one
    hbuf[avg_idx] = h;
    psum += p;
    hsum += h;
    avg_idx = (avg_idx + 1) % AVG_N;
    if (avg_count < AVG_N) avg_count++;
}

void avg_reset(void)
// Resets all variables to zero
{
    for (uint8_t i = 0; i < AVG_N; i++) {
        pbuf[i] = 0;
        hbuf[i] = 0;
    }
    psum = 0;
    hsum = 0;
    avg_idx = 0;
    avg_count = 0;
}

// Averages in ticks; returns 0 if there are no samples yet
float avg_period_ticks(void) { return avg_count ? (float)psum / avg_count : 0.0f; }
float avg_high_ticks(void)   { return avg_count ? (float)hsum / avg_count : 0.0f; }

void exercise_6_1(void) {

	// Calculate the prescaler so it results in a counter clock frequency of 1MHz; i.e. a period of 1uS
	// Use timer as counter
	// TIM2 is 32 bit counter (Counts up to 65536) See Table 5
	// 72MHz / (71 + 1) = 1MHz --> PSC = 71
	// Max time = 2**32 * 1e-6 = 4294.967296s = 71min and 35s

	// 10kHz 50% duty cycle PWM signl as input (See section 21.3.6 in RM, Figure 217)
	// CCR1 captures counter on rising edge
	// CCR2 captures counter on falling edge
	// Counter resets on rising edge

	// CNT frequency = 1MHz / T = 10kHz = 100 counts per period
	//
	// CCR1 effectively measures period --> 100 counts
	// CCR2 effectively measures the high time --> 50 counts

	// Resoluition: 1 tick = 1% of period (good enough?)

	uint32_t p, h;
	TIM2_PWMInput_Init();

	// Copied from exercise 1.3 (EXTI test with buttons and LED)
	initJoystick();
	initLed();
	initEXTI();
	__enable_irq();


	uint8_t running = 1; // flag to indicate that program is running (0 = stop, 1 = running)
	setLed(LED_GREEN);   // start in "running" state
	printf("\r\n--- running ---\r\n");

	while (1) {
	    if (joystick_flag) { // interrupt flag by EXTI
	        joystick_flag = 0;

	        if (current_joystick_state & JOYSTICK_CENTER) {   // pause / resume
	            running = !running;

	            if (running){
	            	printf("\r\n--- resumed ---\r\n");
	            	ledColor = LED_GREEN;
	            }
	            else{
	            	printf("\r\n--- paused ---\r\n");
	            	ledColor = LED_RED;
	            }
	            setLed(ledColor);
	        }

	        if (current_joystick_state & JOYSTICK_DOWN) {     // reset instrument
	            avg_reset();


	            ICValid = 0;
	            printf("\r\n--- reset ---\r\n");
	        }
	    }

	    if (ICValid) {
	        NVIC_DisableIRQ(TIM2_IRQn);
	        p = ICValue1;
	        h = ICValue2;
	        ICValid = 0;
	        NVIC_EnableIRQ(TIM2_IRQn);

	        if (running && p != 0) {
	            setLed(LED_BLUE);            // flash blue when measurement is completed

	            avg_add(p, h);

	            float p_avg = avg_period_ticks() * TIM_TICK_US;   // µs
	            float h_avg = avg_high_ticks()   * TIM_TICK_US;   // µs
	            float f_avg = 1e6f / p_avg;                       // Hz
	            float d_avg = 100.0f * h_avg / p_avg;             // %

	            printf("n=%u  T=%.3f us  high=%.3f us  f=%.3f Hz  D=%.2f %%   \r",
	                   avg_count, p_avg, h_avg, f_avg, d_avg);

	            delay_ms(200);               // throttle output (blue LED stays visible)
	            ICValid = 0;                 // discard captures that arrived during the delay
	            setLed(LED_GREEN);           // back to "running" colour
	        }
	    }
	}
}


// SINGLE CAPTURE
//	            if (p != 0) {
//	                float period_us = p * TIM_TICK_US;
//	                float high_us   = h * TIM_TICK_US;
//	                float freq_hz   = 1e6f / period_us;
//	                float duty_pct  = 100.0f * (float)h / (float)p;
//
//	                printf("T=%.2f us  high=%.2f us  f=%.3f Hz  D=%.1f %%   \r",
//	                       period_us, high_us, freq_hz, duty_pct);
//	            }
