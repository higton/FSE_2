#include <wiringPi.h>
#include <stdio.h>
#include <softPwm.h>

#define RESISTOR_PIN 4
#define COOLER_PIN 5

#define MIN_POWER 40
#define MAX_POWER 100

void config_temperature(){
    if (wiringPiSetup() == -1) {
        printf("Error setting up wiringPi\n");
        return;
    }

    pinMode(RESISTOR_PIN, OUTPUT);
    pinMode(COOLER_PIN, OUTPUT);

    if (softPwmCreate(RESISTOR_PIN, MIN_POWER, MAX_POWER) == -1) {
        printf("Error creating software PWM for resistor\n");
    }
    if (softPwmCreate(COOLER_PIN, MIN_POWER, MAX_POWER) == -1) {
        printf("Error creating software PWM for cooler\n");
    }
}

void change_temperature(double power){
    if(power < 0){
        power *= -1;

        if(power < MIN_POWER) 
            power = MIN_POWER;

        softPwmWrite(COOLER_PIN,power);
        softPwmWrite(RESISTOR_PIN,0);
    } else{
        softPwmWrite(RESISTOR_PIN,power);
        softPwmWrite(COOLER_PIN,0);
    }
}

void close_temp(){
    softPwmWrite(RESISTOR_PIN,0);
    softPwmWrite(COOLER_PIN,0);
}