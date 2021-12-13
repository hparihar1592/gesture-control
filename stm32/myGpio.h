#ifndef __MY_GPIO_H
#define __MY_GPIO_H

#define LCD_LED_Pin GPIO_PIN_10
#define LCD_LED_GPIO_Port GPIOA
#define STEPPER_RST_Pin GPIO_PIN_3
#define STEPPER_RST_GPIO_Port GPIOC
#define STEPPER_VREF_Pin GPIO_PIN_4
#define STEPPER_VREF_GPIO_Port GPIOA
#define STEPPER_ENABLE_Pin GPIO_PIN_4
#define STEPPER_ENABLE_GPIO_Port GPIOC
#define STEPPER_DIRECTION_Pin GPIO_PIN_5
#define STEPPER_DIRECTION_GPIO_Port GPIOC
#define STEPPER_PS_Pin GPIO_PIN_8
#define STEPPER_PS_GPIO_Port GPIOC

void myLCDGpioInit(void);

void myStepperGpioInit(void);

void myDcMotorGpioInit(void);

#endif
