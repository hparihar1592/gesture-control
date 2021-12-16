/*
 *******************************************************************************
 * File Name        :   my_timer.h
 *
 * Description      :   Timer API specification
 *
 * Author          :   Himanshu Parihar
 *
 * Date             :   October 02, 2021
 *******************************************************************************
 */
#ifndef __MY_TIMER_H__
#define __MY_TIMER_H__

#include "stm32f4xx_hal.h"
#include "common.h"
#include <stdbool.h>

typedef struct myTimerDelayGeneratorType {
    uint32_t currentCycleCount;
    uint32_t requiredCycleCount;
    uint32_t remainingPeriod;
    bool lastCycle;
} myTimerDelayGenerator;

/* Init function */
void myTimer2Init(TIM_HandleTypeDef *htim, uint16_t prescaler, uint32_t period);

/* Function to change counter period */
void changeTimer2Period(TIM_HandleTypeDef *htim, uint32_t period);

/* Function to change counter prescaler */
void changeTimer2Prescaler(TIM_HandleTypeDef *htim, uint16_t prescaler);

void changeTimer2CaptureCompare(TIM_HandleTypeDef *htim, uint16_t timChannel, uint32_t value);

/* Function to generate a timer delay and then do something */
void generateTimerNonBlockingDelay(TIM_HandleTypeDef *htim,
                                   uint32_t delay,
                                   myTimerDelayGenerator *delayParameters);

/* Function to generate a timer delay and then do something */
void stopTimerNonBlockingDelay(TIM_HandleTypeDef *htim,
                               myTimerDelayGenerator *delayParameters);

void myTimer1Init(TIM_HandleTypeDef *htim, uint16_t prescaler, uint16_t period);

void myTimer3Init(TIM_HandleTypeDef *htim);

#endif
