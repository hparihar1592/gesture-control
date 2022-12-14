/*
 *******************************************************************************
 * File Name        :   my_timer.c
 *
 * Description      :   Timer API implementation
 *
 * Author           :   Himanshu Parihar
 *
 * Date             :   October 02, 2021
 *******************************************************************************
 */
#include "my_timer.h"
#include "main.h"
#include "stm32f411xe.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sys/_stdint.h"

#define MICROSECONDS_IN_MILLISECONDS (1000)

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

/*
 * Function         :   myTimer2Init
 *
 * Description      :   Initialize Timer 2
 *
 * Parameters       :
 *      prescaler   -   The prescaler value by which timer 2 clock should be divided
 *      period      -   The period value upto which the timer must count
 *
 * Returns          :   void
 */
void myTimer2Init(TIM_HandleTypeDef *htim, uint16_t prescaler, uint32_t period)
{
    // Enable clock
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    htim->Instance = TIM2;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        //        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig) != HAL_OK) {
        //        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) !=
        HAL_OK) {
        //        Error_Handler();
    }

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/*
 * Function         :   changeTimer2CaptureCompare
 *
 * Description      :   Change value of CCR register for timer 2
 *
 * Parameters       :
 *      htim        -   Handle to the timer for which the period must be changed
 *      timChannnel -   Channel number
 *      value       -   New compare value
 *
 * Returns          :   void
 */
void changeTimer2CaptureCompare(TIM_HandleTypeDef *htim, uint16_t timChannel, uint32_t value)
{
    if (htim->Instance != TIM2) {
        return;
    }

    __HAL_TIM_SET_COMPARE(htim, timChannel, value);
}

/*
 * Function         :   changeTimer2Period
 *
 * Description      :   Change the value till which a timer counts before
 *                      resetting
 *
 * Parameters       :
 *      htim        -   Handle to the timer for which the period must be changed
 *      period      -   New period value
 *
 * Returns          :   void
 */
void changeTimer2Period(TIM_HandleTypeDef *htim, uint32_t period)
{
    __HAL_TIM_SET_AUTORELOAD(htim, period);
}

/*
 * Function         :   TIM2_IRQHandler
 * Description      :   This function gets called when an interrupt occurs on
 *                      Timer 2
 *
 * Parameters       :   void
 *
 * Returns          :   void
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/*
 * Function         :   generateTimerNonBlockingFinite
 * Description      :   Call a function after a specified amount of interval
 *                      while also not blocking the CPU
 *
 * Parameters       :
 *      htim        -   Handle to the timer
 *      delay       -   The time in seconds to delay the execution
 *
 * Returns          :   void
 */
void generateTimerNonBlockingDelay(TIM_HandleTypeDef *htim,
                                   uint32_t delay,
                                   myTimerDelayGenerator *delayParameters)
{
    // Reset parameters
    delayParameters->currentCycleCount = 0;
    delayParameters->requiredCycleCount = (delay * MICROSECONDS_IN_MILLISECONDS) / UINT32_MAX;
    delayParameters->remainingPeriod = (delay * MICROSECONDS_IN_MILLISECONDS) % UINT32_MAX;
    delayParameters->lastCycle = false;

    // User provided delay greater than 4 billion, accomodate that
    if (delayParameters->requiredCycleCount > 0) {
        changeTimer2Period(htim, UINT32_MAX - 1);
    } else {
        changeTimer2Period(htim, delayParameters->remainingPeriod - 1);
    }

    HAL_TIM_Base_Start_IT(htim);
}

/*
 * Function         :   stopTimerNonBlocking
 *
 * Description      :   Stop non-blocking delay generated by timer
 *
 * Parameters       :
 *      htim        -   Handle to the timer
 *      delayParams -   Delay parameters structure
 *
 * Returns          :   void
 */
void stopTimerNonBlockingDelay(TIM_HandleTypeDef *htim,
                               myTimerDelayGenerator *delayParameters)
{
    // Reset parameters
    delayParameters->currentCycleCount = 0;
    delayParameters->requiredCycleCount = 0;
    delayParameters->remainingPeriod = 0;
    delayParameters->lastCycle = false;

    changeTimer2Period(htim, 0);

    HAL_TIM_Base_Stop_IT(htim);
}

/*
 * Function         :   myTimer1Init
 *
 * Description      :   Initialize timer 1 for DC motor
 *
 * Parameters       :
 *      htim        -   Handle to timer 1
 *      prescaler   -   Prescaler value to be used
 *      period      -   ARR value to be used
 *
 * Returns          :   void
 */
void myTimer1Init(TIM_HandleTypeDef *htim, uint16_t prescaler, uint16_t period)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

    __HAL_RCC_TIM1_CLK_ENABLE();
    htim->Instance = TIM1;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.RepetitionCounter = 0;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig) != HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }
    if (HAL_TIM_PWM_Init(htim) != HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) !=
        HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    // DC Motor
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }

    // Stepper motor
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(htim, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        printf("Error initializing Timer 1\n");
        return;
    }

    /* TIM1 interrupt Init */
    /*
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    */

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    PB14     ------> TIM1_CH2N
    */
    // DC Motor
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Stepper Motor
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/*
 * Function         :   changeTimer1CaptureCompare
 *
 * Description      :   Change value of CCR register for timer 1
 *
 * Parameters       :
 *      htim        -   Handle to the timer for which the period must be changed
 *      timChannnel -   Channel number
 *      value       -   New compare value
 *
 * Returns          :   void
 */
void changeTimer1CaptureCompare(TIM_HandleTypeDef *htim, uint16_t timChannel, uint16_t value)
{
    if (htim->Instance != TIM1) {
        return;
    }

    __HAL_TIM_SET_COMPARE(htim, timChannel, value);
}

/*
 * Function         :   changeTimer1Period
 *
 * Description      :   Change the value till which a timer counts before
 *                      resetting
 *
 * Parameters       :
 *      htim        -   Handle to the timer for which the period must be changed
 *      period      -   New period value
 *
 * Returns          :   void
 */
void changeTimer1Period(TIM_HandleTypeDef *htim, uint16_t period)
{
    if (htim->Instance != TIM1) {
        return;
    }
    __HAL_TIM_SET_AUTORELOAD(htim, period);
}

/*
 * Function         :   TIM1_UP_TIM10_IRQHandler
 *
 * Description      :   This function gets called when an interrupt occurs on
 *                      Timer 1
 *
 * Parameters       :   void
 *
 * Returns          :   void
 */
/*
void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
*/

/*
 * Function         :   TIM1_CC_IRQHandler
 *
 * Description      :   This function gets called when an interrupt occurs on
 *                      Timer 1
 *
 * Parameters       :   void
 *
 * Returns          :   void
 */
/*
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
*/

/*
 * Function         :   myTimer3Init
 *
 * Description      :   Initialize Timer 3 for Quadrature Encoder
 *
 * Parameters       :
 *      htim        -   Handle to timer 3
 *
 * Returns          :   void
 */
void myTimer3Init(TIM_HandleTypeDef *htim)
{
    // Enable clocks
    __HAL_RCC_TIM3_CLK_ENABLE();

    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim->Instance = TIM3;
    htim->Init.Prescaler = 0;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = UINT16_MAX - 1;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 3;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 3;
    if (HAL_TIM_Encoder_Init(htim, &sConfig) != HAL_OK) {
        printf("Error occured while initializing Timer 3 Encoder\n");
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) != HAL_OK) {
        printf("Error occured while initializing Timer 3\n");
    }

    // Interrupt
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    // GPIOs
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    PA7     ------> TIM3_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*
 * Function         :   TIM3_IRQHandler
 *
 * Description      :   This function gets called when an interrupt occurs on
 *                      Timer 3
 *
 * Parameters       :   void
 *
 * Returns          :   void
 */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}
