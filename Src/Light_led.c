/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * All rights reserved.
 *
 ******************************************************************************
 */

#include <stm32f401re_rcc.h>
#include <stm32f401re_gpio.h>
#include <stm32f401re_adc.h>
#include <stm32f401re_dma.h>
#include <stm32f401re_tim.h>
#include <timer.h>
#include <lightsensor.h>
#include <Ucglib.h>
#include <stdio.h>
#include <string.h>
#include <ucg.h>
#include <led.h>

// macros
#define TIM_PERIOD          8399

#define LED_GPIO_PORT       GPIOA
#define LED_GPIO_PIN        GPIO_Pin_0
#define LED_GPIO_RCC        RCC_AHB1Periph_GPIOA

#define ADC_PORT            GPIOC
#define ADC_PIN             GPIO_Pin_5

#define ADC1_SENSOR         ADC1
#define ADC1_RCC            RCC_APB2Periph_ADC1
#define DMA_CHANNELx        DMA_Channel_0
#define DMA_STREAMx         DMA2_Stream0
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001204C)

// variables
uint16_t uhADCConvertedValue = 0;
static uint32_t time_current, time_initial;
static uint32_t time_total;
static char src1[20] = "";
static char src2[20] = "";
static char src3[20] = "";
static ucg_t ucg;

/*
 * @fn                    - LightSensor_AdcInit
 *
 * @brief                 - This function initializes the ADC with DMA to continuously poll data from the light sensor
 *
 * @param[in]             - ADC_READ_MODE adc_read_mode
 *
 * @return                - none

 * @Note                  - This function sets up for both Polling and DMA mode, in this project we will use Polling Mode
 */
void LightSensor_AdcInit(ADC_READ_MODE adc_read_mode) {
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/* Enable peripheral clocks ***********************************************/
	RCC_APB2PeriphClockCmd(ADC1_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);

	ADC_DeInit();

	/* ADC Common Init ********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init **************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode  = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
		ADC_InitStructure.ADC_DataAlign  = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;
		ADC_Init(ADC1, &ADC_InitStructure);

	if (adc_read_mode == ADC_READ_MODE_POLLING)
	{
		ADC_EOCOnEachRegularChannelCmd(ADC1_SENSOR, ENABLE);
		ADC_ContinuousModeCmd(ADC1_SENSOR, DISABLE);
		ADC_DiscModeChannelCountConfig(ADC1_SENSOR, 1);
		ADC_DiscModeCmd(ADC1_SENSOR, ENABLE);
	}
	else
	{
		/* DMA2_Stream0 channel0 configuration ********************************/
		DMA_DeInit(DMA2_Stream0);
		DMA_InitStructure.DMA_Channel = DMA_CHANNELx;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&uhADCConvertedValue;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA_STREAMx, &DMA_InitStructure);

		/* DMA2_Stream0 enable */
		DMA_Cmd(DMA_STREAMx, ENABLE);

		/* Enable DMA request after last transfer (Single-ADC mode) */
		ADC_DMARequestAfterLastTransferCmd(ADC1_SENSOR, ENABLE);

		/* Enable ADC1 DMA */
		ADC_DMACmd(ADC1_SENSOR, ENABLE);
	}

	/* ADC1 regular channel15 configuration ************************************/
	ADC_RegularChannelConfig(ADC1_SENSOR, ADC_Channel_15, 1, ADC_SampleTime_15Cycles);

	/* Enable ADC1 */
	ADC_Cmd(ADC1_SENSOR, ENABLE);

	if (adc_read_mode == ADC_READ_MODE_DMA)
	{
		/* Start ADC1 Software Conversion */
		ADC_SoftwareStartConv(ADC1_SENSOR);
	}
}

/*
 * @fn                    - LedControl_TimerOCInit
 *
 * @brief                 - This function has a timer initialization function to control
 *                          the LEDs using the PWM (Pulse Width Modulation) method
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
static void LedControl_TimerOCInit(void)
{
    GPIO_InitTypeDef           GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef    Timer_InitStructure;
    TIM_OCInitTypeDef          TIM_OCInitStructure;

    // Enable GPIOA Clock
    RCC_AHB1PeriphClockCmd(LED_GPIO_RCC, ENABLE);

    // Configure GPIO Pin for PWM output
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);

    GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

    // Enable Timer1
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Timer Base Init
    Timer_InitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    Timer_InitStructure.TIM_Prescaler         = TIM_PERIOD;
    Timer_InitStructure.TIM_Period            = 999;
    Timer_InitStructure.TIM_ClockDivision     = 0;
    Timer_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &Timer_InitStructure);

    // PWM Mode Config
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse       = 999/2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_Cmd(TIM2, ENABLE);
    TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

/*
 * @fn                    - LightSensor_AdcPollingRead
 *
 * @brief                 - This function has the function of reading the value from the light sensor through
 *                          the ADC (Analog-to-Digital Converter) of the microcontroller using the polling method.
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
uint16_t
LightSensor_AdcPollingRead (void)
{
	uint16_t result = 0;
	ADC_SoftwareStartConv(ADC1_SENSOR);
	while (ADC_GetFlagStatus(ADC1_SENSOR, ADC_FLAG_EOC) == RESET);
	result = ADC_GetConversionValue(ADC1_SENSOR);
	return result;
}

/*
 * @fn                    - LedControl_TimerOCSetPwm
 *
 * @brief                 - This function has the function of controlling the brightness of the LED
 *                          by setting the pulse width (PWM) based on the input duty cycle value.
 *
 * @param[in]             - uint8_t dutyCycle
 *
 * @return                - none

 * @Note                  - none
 */
static void LedControl_TimerOCSetPwm (uint8_t dutyCycle)
{
	static uint32_t pulse_length = 0;

	if(dutyCycle >= 100) return;

	pulse_length = ((TIM_PERIOD * dutyCycle) / 1000);// value 0- 8399 depend on duty cycle
	memset(src2, 0, sizeof(src2));
	sprintf(src2,"Light = %ld lux",pulse_length);
	ucg_DrawString(&ucg, 0, 32, 0, src2);

	TIM_SetCompare1(TIM2,pulse_length);
}

/*
 * @fn                    - ABL_StepBrightness
 *
 * @brief                 - This function check ADC value and update PWM for LED brightness
                            and adjust brightness based on ADC light sensor reading

 * @param[in]             - uint16_t AdcValue
 *
 * @return                - none

 * @Note                  - none
 */
static void ABL_StepBrightness(uint16_t AdcValue) {
	uint8_t dutyCycle;

    // Map ADC value to PWM duty cycle (0 - 100%)
    // Assuming ADC max value is 4095 for a 12-bit resolution

    dutyCycle = (AdcValue * 100) / 4095;
    memset(src3, 0, sizeof(src3));
    sprintf(src3,"Light = %d lux",dutyCycle);
    ucg_DrawString(&ucg, 0, 72, 0, src3);

    // Set PWM duty cycle to control LED brightness
    LedControl_TimerOCSetPwm(dutyCycle);
}

/*
 * @fn                    - ABL_Process
 *
 * @brief                 - Process function to read light sensor and control LED brightness every 100ms

 * @param[in]             - uint16_t AdcValue
 *
 * @return                - none

 * @Note                  - none
 */
void ABL_Process(void) {
    uint16_t AdcValue;
    time_current= GetMilSecTick();

    	if(time_current >= time_initial){
    		time_total += time_current - time_initial;
    	}

    	else
    	{
    		time_total += 0xFFFFFFFFU - time_current + time_initial;
    	}
    // Execute every 100 ms
    if (time_total >= 100) {
        time_total = 0;

        // Read ADC value from light sensor
        AdcValue = (uint16_t)LightSensor_AdcPollingRead();
        memset(src1, 0, sizeof(src1));
        sprintf(src1,"Light = %d lux",AdcValue );
        ucg_DrawString(&ucg, 0, 52, 0, src1);

        // Adjust LED brightness based on the ADC value
        ABL_StepBrightness(AdcValue);
    }
    time_initial = time_current;
}

/*
 * @fn                    - main
 *
 * @brief                 - This function is reponsible for the running of the program
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
int main(void)
{
    SystemCoreClockUpdate();
    TimerInit();
    LightSensor_AdcInit(ADC_READ_MODE_POLLING);
    LedControl_TimerOCInit();
    Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);
    ucg_ClearScreen(&ucg);
    ucg_SetFont(&ucg, ucg_font_ncenB08_hr);
    ucg_SetColor(&ucg, 0 , 255, 255, 255);
    ucg_SetColor(&ucg, 1, 0, 0, 0);
    ucg_SetRotate180(&ucg);
    time_initial = time_current;
    LedControl_Init();

    while (1)
    {
        processTimerScheduler();
        ABL_Process();
    }
}
