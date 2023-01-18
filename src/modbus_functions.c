#include <stdio.h>
#include <string.h>

#include "modbus_functions.h"
#include "modbus.h"

enum {
    GET_DATA = 0x23,
    SEND_DATA = 0x16
};

enum {
    INTERNAL_TEMP = 0xc1,
    POTENTIOMETER_TEMP = 0xc2,
    USER_COMMAND = 0xc3,
    INT_CONTROL = 0xd1,
    FLOAT_CONTROL = 0xd2,
    SYSTEM_STATUS = 0xd3,
    SYSTEM_MODE = 0xd4,
    FUNCTIONING_STATE = 0xd5,
    AMBIENT_TEMP = 0xd6
};

float get_temperature(unsigned char device) {
    modbus_config(GET_DATA, device);
    modbus_send(NULL, 0);
    unsigned char message[255];

    int size = modbus_receive(message, 254);
    if (size < 0) {
        printf("Error receiving message\n");
        modbus_close();
        return -1;
    }

    float temp = *(float *) &message[3];
    modbus_close();
    return temp;
}

float get_internal_temperature() {
    return get_temperature(INTERNAL_TEMP);
}

float get_potentiometer_temperature() {
    return get_temperature(POTENTIOMETER_TEMP);
}

int get_user_command() {
    modbus_config(GET_DATA, USER_COMMAND);
    modbus_send(NULL, 0);
    unsigned char message[255];

    int size = modbus_receive(message, 254);
    if (size < 0) {
        printf("Error receiving message\n");
        modbus_close();
        return -1;
    }

    int command = message[3];
    modbus_close();
    switch (command) {
        case 0xA1:
            printf("COMMAND RECEIVED: TURN ON\n");
            send_system_state(1);
            return 1;
            break;
        case 0xA2:
            printf("COMMAND RECEIVED: TURN OFF\n");
            send_system_state(0);
            return 2;
            break;
        case 0xA3:
            printf("COMMAND RECEIVED: START\n");
            send_functioning_state(1);
            return 3;
            break;
        case 0xA4:
            printf("COMMAND RECEIVED: STOP\n");
            send_functioning_state(0);
            return 4;
            break;
        case 0xA5:
            printf("COMMAND RECEIVED: CHANGE MODE\n");
            return 5;
            break;
    }
    return 0;
}

void send_int_control(int control_signal){
    modbus_config(SEND_DATA, INT_CONTROL);
    unsigned char message[255];
    memcpy(message, &control_signal, sizeof(int));

    modbus_send(message, 4);
    modbus_close();
}

void send_float_control(float control_signal){
    modbus_config(SEND_DATA, FLOAT_CONTROL);
    unsigned char message[255];
    memcpy(message, &control_signal, sizeof(float));

    modbus_send(message, 4);
    modbus_close();
}

void send_system_state(int control_signal){
    modbus_config(SEND_DATA, SYSTEM_STATUS);
    unsigned char message[255];
    memcpy(message, &control_signal, sizeof(int));

    modbus_send(message, 1);
    int size = modbus_receive(message, 254);
    if(size == -1){
        printf("INVALID CRC\n");
    }

    modbus_close();
}

void send_system_mode(int control_signal){
    modbus_config(SEND_DATA, SYSTEM_MODE);
    unsigned char message[255];
    memcpy(message, &control_signal, sizeof(int));

    modbus_send(message, 1);
    modbus_close();
}

void send_functioning_state(int control_signal){
    modbus_config(SEND_DATA, FUNCTIONING_STATE);
    unsigned char message[255];
    memcpy(message, &control_signal, sizeof(int));

    modbus_send(message, 1);
    modbus_close();
}

void send_ambient_temperature(float temperature){
    modbus_config(SEND_DATA, AMBIENT_TEMP);
    unsigned char message[255];
    memcpy(message, &temperature, sizeof(float));

    modbus_send(message, 4);
    modbus_close();
}
