#include "timer.h"
#include "stm32f30x.h"
#include "gpio.h"

#define TIM_TICK_US  1.0f    // (PSC + 1) / 72 MHz = 1 micro second

volatile uint32_t ICValue1, ICValue2; // Initialize variables to store last readings of the CCR1 and CCR2
volatile uint8_t  ICValid = 0; // Flag to see if new reading is available

void TIM2_IRQHandler(void) {
	// ISR = Interrupt Service Routine.
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

    ICValue1 = TIM_GetCapture1(TIM2); // Period from CCR1
    ICValue2 = TIM_GetCapture2(TIM2); // Width from CCR2
    ICValid = 1;
}

// Code for calculating running average
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
	            	setLed(LED_GREEN);
	            }
	            else{
	            	printf("\r\n--- paused ---\r\n");
	            	setLed(LED_RED);
	            }

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

//	        if (running && p != 0) {
//	            setLed(LED_BLUE);            // flash blue when measurement is completed
//
//	            avg_add(p, h);
//
//	            float p_avg = avg_period_ticks() * TIM_TICK_US;   // µs
//	            float h_avg = avg_high_ticks()   * TIM_TICK_US;   // µs
//	            float f_avg = 1e6f / p_avg;                       // Hz½
//	            float d_avg = 100.0f * h_avg / p_avg;             // %
//
//	            printf("n=%u  T=%.3f us  high=%.3f us  f=%.3f Hz  D=%.2f %%   \r",
//	                   avg_count, p_avg, h_avg, f_avg, d_avg);
//	            ICValid = 0;                 // discard captures that arrived during the delay
//	        }
			if (running && p != 0) {
				float period_us = p * TIM_TICK_US;
				float high_us   = h * TIM_TICK_US;
				float freq_hz   = 1e6f / period_us;
				float duty_pct  = 100.0f * (float)h / (float)p;

				printf("T=%.2f us  high=%.2f us  f=%.3f Hz  D=%.1f %%   \r",
					   period_us, high_us, freq_hz, duty_pct);
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
