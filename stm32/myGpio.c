#include "myGpio.h"
#include "common.h"
#include "stm32f4xx_hal_gpio.h"

void myLCDGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure GPIO pin output level */
    HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET);

    /* Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = LCD_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STEPPER_VREF_GPIO_Port, &GPIO_InitStruct);
}

void myStepperGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, STEPPER_RST_Pin | STEPPER_PS_Pin, GPIO_PIN_SET);

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(STEPPER_VREF_GPIO_Port, STEPPER_VREF_Pin, GPIO_PIN_SET);

    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, STEPPER_ENABLE_Pin | STEPPER_DIRECTION_Pin,
                      GPIO_PIN_RESET);

    /* Configure GPIO pins : PCPin PCPin */
    GPIO_InitStruct.Pin = STEPPER_RST_Pin | STEPPER_PS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = STEPPER_VREF_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STEPPER_VREF_GPIO_Port, &GPIO_InitStruct);

    /* Configure GPIO pins : PCPin PCPin */
    GPIO_InitStruct.Pin = STEPPER_ENABLE_Pin | STEPPER_DIRECTION_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
}

void myDcMotorGpioInit(void)
{
    return;
}
