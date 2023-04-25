#ifndef _RADIO_H_
#define _RADIO_H_

#define CE_PIN GPIO_NUM_27
#define CSN_PIN GPIO_NUM_5

struct Joystick
{
    float angX;
    float angY;
    uint8_t sw;
};

void vTaskRadio(void *params);
#endif