#pragma once

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

// Buttons are pull down or pulled up.
// 0 logic is taking them down to 0v
// 1 logic is taking them to 3.3v


#define GPIO_PIN_MENU 97 // PIN_11: bank 3 (x32), index 1 (+1)
#define GPIO_PIN_BACK 98 // PIN_13
#define GPIO_PIN_PLUS 105 // PIN_16
#define GPIO_PIN_MINUS 106 // PIN_18


#define GPIO_PIN_QACTION1 114 // PIN_32
#define GPIO_PIN_QACTION2 102 // PIN_38
#define GPIO_PIN_QACTION2_2 -1
#define GPIO_PIN_QACTION3 101 //PIN_40
#define GPIO_PIN_QACTIONPLUS -1
#define GPIO_PIN_QACTIONMINUS -1



#define GPIO_PIN_LED_RED 108 // PIN_31
#define GPIO_PIN_LED_GREEN 107 // PIN_29
#define GPIO_PIN_BUZZER -1


#ifdef __cplusplus
extern "C" {
#endif 

int GPIOGetPinLedRed();
int GPIOGetPinLedGreen();
int GPIOGetPinBuzzer();
int GPIOGetPinMenu();
int GPIOGetPinBack();
int GPIOGetPinPlus();
int GPIOGetPinMinus();
int GPIOGetPinQA1();
int GPIOGetPinQA2();
int GPIOGetPinQA22();
int GPIOGetPinQA3();
int GPIOGetPinQAPlus();
int GPIOGetPinQAMinus();

int GPIOExport(int pin);
int GPIOUnexport(int pin);
int GPIODirection(int pin, int dir);
int GPIORead(int pin);
int GPIOWrite(int pin, int value);

int GPIOInitButtons();
int GPIOGetButtonsPullDirection();

#ifdef __cplusplus
}  
#endif 
