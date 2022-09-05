/*
 *******************************************************************************
 * File Name        :   final_project.c
 *
 * Description      :   Motor Gesture Control
 *
 * Author           :   Himanshu Parihar
 *
 * Date             :   December 01, 2021
 *******************************************************************************
 */
#include <stdio.h>
#include "common.h"
#include "main.h"
#include "my_gpio.h"
#include "my_timer.h"
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#include "HD44780_F3.h"
#include "my_defines.h"
#include "sys/_stdint.h"
#include <math.h>
#include <inttypes.h>

#define STEPPER_ONE_REVOLUTION_MICROSTEPS 1600
#define QUADRATURE_ONE_REVOLUTION_VALUE 800
#define SAMPLE_TIME 500    // milliseconds
#define SAMPLE_MULTIPLIER (1000 / SAMPLE_TIME)
#define STEPPER_MAX_SPEED_ARR 20000
#define STEPPER_MIN_SPEED_ARR (UINT16_MAX - 1)

typedef enum {
    STEPPER,
    DC
} MotorType;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

volatile uint32_t currentTime = 0;
volatile uint32_t previousTime = 0;
volatile uint32_t timeElapsed = 0;

MotorType currentMotorType = STEPPER;

myTimerDelayGenerator delayParams = {
    .currentCycleCount = 0,
    .requiredCycleCount = 0,
    .remainingPeriod = 0,
    .lastCycle = false
};

int16_t currentEncoderValue = 0;
int16_t lastEncoderValue = 0;
uint16_t currentDcRpm = 0;
uint16_t currentStepperRpm = 0;

/*
 * Function         :   CmdInit
 *
 * Description      :   Initialize peripherals for feedback loop
 *
 * Parameters       :
 *      action      -   Integer indicating action type
 *
 * Returns          :   ParserReturnVal_t indicating OK or Failure
 */
ParserReturnVal_t CmdInit(int action)
{
    if (action == CMD_SHORT_HELP) {
        return CmdReturnOk;
    }
    if (action == CMD_LONG_HELP) {
        printf("Initialize Timer 1 for PWM\n");
        return CmdReturnOk;
    }

    // LCD
    myLCDGpioInit();
    HD44780_Init();
    HD44780_ClrScr();
    HD44780_PutStr("Gesture Control");

    // Stepper Motor
    myStepperGpioInit();

    // DC Motor
    myDcMotorGpioInit();

    // Steper Motor and DC Motor
    myTimer1Init(&htim1, 1 - 1, UINT16_MAX - 1);

    // Encoder
    myTimer3Init(&htim3);
    HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);

    // Delay at regular intervals
    myTimer2Init(&htim2, 100 - 1, UINT32_MAX - 1);
    generateTimerNonBlockingDelay(&htim2, SAMPLE_TIME, &delayParams);

    // Analog interface
    myAnalogGpioInit();
    HAL_GPIO_WritePin(ANALOG_INTERFACE_TEST_GPIO_Port, ANALOG_INTERFACE_TEST_Pin, GPIO_PIN_SET);

    return CmdReturnOk;
}

/*
 * Function         :   CmdStop
 *
 * Description      :   Stop motors
 *
 * Parameters       :
 *      action      -   Integer indicating action type
 *
 * Returns          :   ParserReturnVal_t indicating OK or Failure
 */
ParserReturnVal_t CmdStop(int action)
{
    if (action == CMD_SHORT_HELP) {
        return CmdReturnOk;
    }
    if (action == CMD_LONG_HELP) {
        printf("Stop DC and Stepper motor\n");
        return CmdReturnOk;
    }

    // Stop dc and stepper motor
    HAL_TIMEx_PWMN_Stop(&htim1, STEPPER_MOTOR_TIMER_CHANNEL);
    HAL_TIM_PWM_Stop(&htim1, DC_MOTOR_TIMER_CHANNEL);

    // Reset LCD
    HD44780_ClrScr();
    HD44780_PutStr("Gesture Control");

    return CmdReturnOk;
}

/*
 * Function         :   HAL_TIM_PeriodElapsedCallback
 *
 * Description      :   This function gets called whenever an interrupt
 *                      occurs after the ARR register in a timer has reached its
 *                      defined value
 *
 * Parameters       :   void
 *      htim        -   Handle to the timer for which the interrupt is generated
 *
 * Returns          :   void
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Time counter
    if (htim == &htim2) {
        if (delayParams.lastCycle) {
            // Looping
            delayParams.currentCycleCount = 0;

            // Reset the period to max
            if (delayParams.requiredCycleCount > 0) {
                delayParams.lastCycle = false;
                changeTimer2Period(htim, UINT32_MAX - 1);
            }

            /* The below instructions are ran at the end of each delay */
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);    // Board LED to view delays

            // Calculate DC RPM
            currentEncoderValue = (int16_t) __HAL_TIM_GET_COUNTER(&htim3);
#ifdef DEBUG
            printf("Current encoder value after 100ms = %" PRId16 "\n",
                   currentEncoderValue);
#endif

            // Maximum difference between current encoder reading and previous
            // encoder reading does not cross values greater than 10,000 at the
            // max rated RPM Hence if difference is greater than 10,000, an
            // overflow of CNT register occured
            if (abs(currentEncoderValue - lastEncoderValue) > 10000) {
                // Wrapped around, leave this iteration
                lastEncoderValue = currentEncoderValue;
#ifdef DEBUG
                printf("Wrapped around, skipping.\n");
#endif
                return;
            } else {
                currentDcRpm = (abs(currentEncoderValue - lastEncoderValue) * SAMPLE_MULTIPLIER * 60) / QUADRATURE_ONE_REVOLUTION_VALUE;
#ifdef DEBUG
                printf("current rpm: %" PRIu16 "\n", currentRPM);
#endif
            }
            // Update variables for RPM calculation
            lastEncoderValue = currentEncoderValue;

            // Stepper RPM
            uint16_t stepperARR = __HAL_TIM_GET_AUTORELOAD(&htim1);
            if (stepperARR < STEPPER_MAX_SPEED_ARR - 1 || stepperARR > STEPPER_MIN_SPEED_ARR) {
                currentStepperRpm = 0;
            } else {
                currentStepperRpm = 60.0 / ((double) stepperARR * 10 * 1600 / 1000000000.0);
            }
        } else if (delayParams.currentCycleCount ==
                   delayParams.requiredCycleCount) {
            // Last cycle remaining
            changeTimer2Period(htim, delayParams.remainingPeriod - 1);
            delayParams.lastCycle = true;
        } else {
            delayParams.currentCycleCount++;
        }
    }
}

ADD_CMD("init", CmdInit, "Initialize peripherals")
ADD_CMD("stop", CmdStop, "Stop motor rotation")
