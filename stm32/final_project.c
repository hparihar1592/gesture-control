/*
 *******************************************************************************
 * File Name        :   final_project.c
 *
 * Description      :   TODO
 *
 * Author           :   Himanshu Parihar
 *
 * Date             :   October 02, 2021
 *******************************************************************************
 */
#include <stdio.h>
#include "common.h"
#include "myGpio.h"
#include "my_timer.h"
#include "stdint.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#include "HD44780_F3.h"

#define DC_MOTOR_TIMER_CHANNEL TIM_CHANNEL_1
#define STEPPER_MOTOR_TIMER_CHANNEL TIM_CHANNEL_2

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/*
 * Function         :   CmdLoopInit
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
    HD44780_PutStr("Yo World");

    // Stepper Motor
    myStepperGpioInit();
    myTimer1Init(&htim1, 1 - 1, UINT16_MAX - 1);
    HAL_TIMEx_PWMN_Start(&htim1, STEPPER_MOTOR_TIMER_CHANNEL);

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

    HAL_TIMEx_PWMN_Stop(&htim1, STEPPER_MOTOR_TIMER_CHANNEL);

    return CmdReturnOk;
}

ADD_CMD("init", CmdInit, "Initialize peripherals")
ADD_CMD("stop", CmdStop, "Stop motor rotation")
