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
#include <system_stm32f4xx.h>
#include <stm32f401re_rcc.h>
#include <stm32f401re_gpio.h>
#include <stm32f401re_adc.h>
#include <stm32f401re_dma.h>
#include <stm32f401re_tim.h>
#include <stm32f401re_i2c.h>
#include <stm32f401re_spi.h>
#include <buzzer.h>
#include <melody.h>
#include <timer.h>
#include <lightsensor.h>
#include <temhumsensor.h>
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

#define SPI1_CS_PORT         GPIOB
#define SPI1_CS_PIN          GPIO_Pin_6
#define SPI1_RST_PORT        GPIOC
#define SPI1_RST_PIN         GPIO_Pin_7
#define SPI1_MOSI_PORT       GPIOA
#define SPI1_MOSI_PIN        GPIO_Pin_7
#define SPI1_SCK_PORT        GPIOA
#define SPI1_SCK_PIN         GPIO_Pin_5
#define SPI1_RS_PORT         GPIOA
#define SPI1_RS_PIN          GPIO_Pin_9
#define SPI1_ENABLE_PORT     GPIOB
#define SPI1_ENABLE_PIN      GPIO_Pin_10
#define SPI1_MODE_PORT       GPIOA
#define SPI1_MODE_PIN        GPIO_Pin_8

#define TEMP_HUM_ADR            0x40
#define reg_Temp_Addr           0xE3
#define reg_Hum_Addr            0xE5

// variables
uint16_t uhADCConvertedValue = 0;
static uint32_t time_current, time_initial;
static uint32_t time_total;
static char src1[20] = "";
static char src2[20] = "";
static char src3[20] = "";
static char src4[20] = "";
static ucg_t ucg;
static uint8_t temperature, humidity;

/*
 * @fn                    - SPI1_Init
 *
 * @brief                 - This function is responsible for initializing SPI1 to communicate with LCD
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - SPI makes it easy and fast to transmit short or continuous data.
 *                          The LCD can receive data from STM32F401 through MOSI
 *                          and control the transmission process through SCK and CS.
 */
void SPI1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef SPI_InitStruct;

    // 1. Enable SPI1 and GPIO clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // GPIOA for SCK, MOSI
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // GPIOB for CS, RST

    // 2. Configure SCK (PA5)
    GPIO_InitStruct.GPIO_Pin = SPI1_SCK_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(SPI1_SCK_PORT, GPIO_PinSource5, GPIO_AF_SPI1);  // Connect PA5 to SPI1_SCK

    // 3. Configure MOSI (PA7)
    GPIO_InitStruct.GPIO_Pin = SPI1_MOSI_PIN;
    GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(SPI1_MOSI_PORT, GPIO_PinSource7, GPIO_AF_SPI1);  // Connect PA7 to SPI1_MOSI

    // 4. Configure CS (PB6) as GPIO for manual chip select
    GPIO_InitStruct.GPIO_Pin = SPI1_CS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(SPI1_CS_PORT, &GPIO_InitStruct);
    GPIO_SetBits(SPI1_CS_PORT, SPI1_CS_PIN);  // Set CS high (inactive)

    // 5. Configure SPI1
    SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx; // 1-line transmit
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;  // Software managed NSS
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStruct);

    // 6. Enable SPI1
    SPI_Cmd(SPI1, ENABLE);
}


/*
 * @fn                    - I2C1_Init
 *
 * @brief                 - This function is reponsible for initializing I2C to communicate with TemHumSensor
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
void I2C1_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure I2C SCL and SDA
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;  // SCL (PB6), SDA (PB9)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // I2C Init
    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    // Enable I2C
    I2C_Cmd(I2C1, ENABLE);
}

/*
 * @fn                    - I2C_Start
 *
 * @brief                 - This function is reponsible for starting I2C communication in Master mode
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
static void I2C_Start(void)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // Master se khong truyen du lieu cho den khi slave ranh roi

	I2C_GenerateSTART(I2C1, ENABLE);

	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)); // Chuyen qua che do Master
}

/*
 * @fn                    - I2C_SendAddress
 *
 * @brief                 - This function is reponsible for sending the address of the slave
 *                          along with the mode of transmitting or receiving data
 *
 * @param[in]             - uint8_t address, uint8_t direction
 *
 * @return                - none

 * @Note                  - none
 */
static void I2C_SendAddress(uint8_t address, uint8_t direction)
{
		I2C_Send7bitAddress(I2C1, address, direction);

		if (direction == I2C_Direction_Transmitter)
		{
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
		}
		else if (direction == I2C_Direction_Receiver)
		{
			while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
		}
}

/*
 * @fn                    - I2C_TransmitData
 *
 * @brief                 - This function is reponsible for sending a byte of data from the Master to the Slave.
 *
 * @param[in]             - uint8_t data
 *
 * @return                - none

 * @Note                  - none
 */
static void I2C_TransmitData(uint8_t data)
{
	I2C_SendData(I2C1, data);

	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/*
 * @fn                    - I2C_Stop
 *
 * @brief                 - This function is reponsible for ending the I2C prosses
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */

static void I2C_Stop(void)
{
	I2C_GenerateSTOP(I2C1, ENABLE);
}
/*
 * @fn                    - I2C_ReceiveData_ACK
 *
 * @brief                 - This function is reponsible for receiveing a byte of data from
 *                          the Slave and sends an ACK signal.
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
static uint8_t I2C_ReceiveData_ACK(void) {
    // Enable ACK for receiving more data
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    // Wait until data is received
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

    // Read and return the received data
    return I2C_ReceiveData(I2C1);
}

/*
 * @fn                    - I2C_ReceiveData_NACK
 *
 * @brief                 - This function is reponsible for receiveing a byte of data from
 *                          the Slave and not send an ACK signal
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
static uint8_t I2C_ReceiveData_NACK(void) {
    // Disable ACK for the last byte
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    // Wait until data is received
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));

    // Read and return the received data
    return I2C_ReceiveData(I2C1);
}


/*
 * @fn                    - TemHumSensor_readRegister
 *
 * @brief                 - This function is reponsible for reading data from temperature
 *                          and humidity sensors via I2C communication
 *
 * @param[in]             - uint8_t deviceAddr, uint8_t* pAddressReg, uint8_t* pDataRead, uint8_t byLengthData
 *
 * @return                - none

 * @Note                  - none
 */
void TemHumSensor_readRegister(uint8_t deviceAddr, uint8_t* pAddressReg, uint8_t* pDataRead, uint8_t byLengthData) {
    uint8_t byLenCmd = pAddressReg[0];
    I2C_Start();
    I2C_SendAddress(TEMP_HUM_ADR << 1, I2C_Direction_Transmitter);
    for (uint8_t i = 1; i < byLenCmd; i++) {
    	I2C_TransmitData(pAddressReg[i]);
    }


    I2C_Stop();
    I2C_Start();

    I2C_SendAddress(deviceAddr << 1,I2C_Direction_Receiver);

    for (uint8_t i = 0; i < byLengthData; i++) {
        if (i == (byLengthData - 1)) {
            pDataRead[i] = I2C_ReceiveData_NACK();  // Không gửi ACK cho byte cuối
        } else {
            pDataRead[i] = I2C_ReceiveData_ACK();   // Gửi ACK cho các byte khác
        }
    }
    I2C_Stop();
}


/*
 * @fn                    - TemHumSensor_getTemperature
 *
 * @brief                 - This function helps to obtain and process temperature data from the SI7020 sensor.
 *
 * @param[in]             - uint8_t deviceAddr
 *
 * @return                - float temperature

 * @Note                  - The equation to calculate temperature is in the datasheet of SI7020
 */
float TemHumSensor_getTemperature(uint8_t deviceAddr) {
    uint8_t tempData[2];
    uint8_t TEMP[2] =  {2, 0xE3};
    TemHumSensor_readRegister(deviceAddr, TEMP, tempData, 2);

    uint16_t rawTemperature = (tempData[0] << 8) | tempData[1];
    float temperature = -46.85 + 175.72 * (rawTemperature / 65536.0);

    return temperature;
}

/*
 * @fn                    - TemHumSensor_getHumidity
 *
 * @brief                 - This function helps to obtain and process humidity data from the SI7020 sensor.
 *
 * @param[in]             - uint8_t deviceAddr
 *
 * @return                - float humidity

 * @Note                  - The equation to calculate humidity is in the datasheet of SI7020
 */
float TemHumSensor_getHumidity(uint8_t deviceAddr) {
    uint8_t humData[2];
    uint8_t TEMP[2] =  {2, 0xE5};
    TemHumSensor_readRegister(deviceAddr, TEMP, humData, 2);

    uint16_t rawHumidity = (humData[0] << 8) | humData[1];
    float humidity = -6 + 125.0 * (rawHumidity / 65536.0);

    return humidity;
}

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

    dutyCycle = 100 - ((AdcValue * 100) / 4095);

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
    uint16_t light;
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
        light = (uint16_t)LightSensor_AdcPollingRead();
        memset(src1, 0, sizeof(src1));
        sprintf(src1,"Light = %d lux",light );
        ucg_DrawString(&ucg, 0, 32, 0, src1);

        temperature = (uint8_t)(TemHumSensor_getTemperature(TEMP_HUM_ADR));
        humidity = (uint8_t)(TemHumSensor_getHumidity(TEMP_HUM_ADR));

        memset(src3, 0, sizeof(src3));
        sprintf(src3, "Temp = %d oC", temperature);
        ucg_DrawString(&ucg, 0, 52, 0, src3);

        memset(src2, 0, sizeof(src2));
        sprintf(src2, "Humi = %d %%", humidity);
        ucg_DrawString(&ucg, 0, 72, 0, src2);

        if((humidity < 40 || humidity > 70) && (temperature > 35)){
        	LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_RED, 50);
        	LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_RED, 50);
        	BuzzerControl_SetMelody(pbeep);
        }
        else if ( temperature == 35){
        	memset(src4, 0, sizeof(src4));
        	sprintf(src1,"Dieu kien toi uu");
        	ucg_DrawString(&ucg, 0, 92, 0, src4);
        }
        else {
        	LedControl_SetColorGeneral(LED_KIT_ID0, LED_COLOR_BLUE, 50);
        	LedControl_SetColorGeneral(LED_KIT_ID1, LED_COLOR_BLUE, 50);
        }


        // Adjust LED brightness based on the ADC value
        ABL_StepBrightness(light);
    }
    time_initial = time_current;
}

/*
 * @fn                    - AppInitCommon
 *
 * @brief                 - This function is initialize functions which using timer, buzzer, ...
 *
 * @param[in]             - none
 *
 * @return                - none

 * @Note                  - none
 */
void AppInitCommon(void){
	SystemCoreClockUpdate();
	TimerInit();
	LightSensor_AdcInit(ADC_READ_MODE_POLLING);
	LedControl_TimerOCInit();
	SPI1_Init();
	I2C1_Init();
	time_initial = GetMilSecTick();
	BuzzerControl_Init();
	TemHumSensor_Init();
	Ucglib4WireSWSPI_begin(&ucg, UCG_FONT_MODE_SOLID);
	ucg_ClearScreen(&ucg);
	ucg_SetFont(&ucg, ucg_font_ncenB08_hr);
	ucg_SetColor(&ucg, 0 , 255, 255, 255);
	ucg_SetColor(&ucg, 1, 0, 0, 0);
	ucg_SetRotate180(&ucg);
	time_initial = time_current;
	LedControl_Init();
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
	AppInitCommon();
    while (1)
    {
        processTimerScheduler();
        ABL_Process();
    }
}
