#include <Arduino.h>
#include <PCA9685.h>
#include <Ramp.h>
#include "Servos.h"
#include "main.h"

PCA9685 pwmController; // Library using default B000000 (A5-A0) i2c address, and default Wire @400kHz
rampInt servoAngles;

void vTaskServos(void *params)
{
    pwmController.resetDevices();    // Resets all PCA9685 devices on i2c line
    pwmController.init();            // Initializes module using default totem-pole driver mode, and default disabled phase balancer
    pwmController.setPWMFreqServo(); // 50Hz provides standard 20ms servo phase length

    struct Params *p = (struct Params *)params;

    servoAngles.go(0); // https://github.com/siteswapjuggler/RAMP/blob/master/examples/ramp_servo/ramp_servo.ino

    while (true)
    {
        // pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(map(perro.pos1, 0, 180, -90, 90)));
        // pwmController.setChannelPWM(0, servos.frontLeft.shoulder.pwmForAngle(map(perro.frontLeft.shoulder, 0, 180, -90, 90)));
        // pwmController.setChannelPWM(0, servos.frontLeft.shoulder.pwmForAngle(map(perro.pos1, 0, 180, -90, 90)));
        // pwmController.setChannelPWM(1, pwmServo2.pwmForAngle(map(perro.pos2, 0, 180, -90, 90)));
        // pwmController.setChannelPWM(2, pwmServo3.pwmForAngle(map(perro.pos3, 0, 180, -90, 90)));

        pwmController.setChannelPWM(0, p->servos.frontLeft.shoulder.pwmForAngle(map(p->perro.frontLeft.shoulder, 0, 180, -90, 90)));
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}