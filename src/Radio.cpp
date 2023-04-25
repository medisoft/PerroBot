#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#include "Radio.h"
#include "main.h"

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN
const byte identificacion[6] = "00001";

void vTaskRadio(void *params)
{
    while (!radio.begin())
    {
        Serial.println("No pude detectar el dispositivo radio");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    };
    radio.openReadingPipe(0, identificacion);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();

    struct Params *p = (struct Params *)params;

    while (true)
    {
        if (radio.available())
        {
            // char texto[32] = "";
            // radio.read(&texto, sizeof(texto));
            // Serial.print("Recibi:");
            // Serial.println(texto);
            Joystick jy;
            radio.read(&jy, sizeof(jy));

            Serial.print("VRx:");
            Serial.print(jy.angX);
            Serial.print(",VRy:");
            Serial.print(jy.angY);
            Serial.print(",SW:");
            Serial.print(jy.sw);
            Serial.println();
            // p->perro.frontLeft.shoulder = jy.angX;
            // perro->pos1 = jy.angX;
            // perro->pos2 = jy.angY;

            if (jy.sw == 1)
            {
                pinMode(GPIO_NUM_14, INPUT);
                // attachInterrupt(digitalPinToInterrupt(GPIO_NUM_14), isrCallbackFunction, FALLING);
                esp_sleep_enable_timer_wakeup(30000000);
                esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, HIGH);
                delay(1000);
                radio.flush_rx();
                esp_deep_sleep_start();
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}