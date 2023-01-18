#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "modbus.h"
#include "crc16.h"
#include "uart_file.h"

#define SLEEP_TIME 100
#define TIMEOUT 5

static unsigned char device_code = 0x01;
static unsigned char matricula[] = {4, 5, 5, 4};
unsigned char func_code = 0x00;
unsigned char func_sub_code = 0x00;

int modbus_config(unsigned char update_func_code, unsigned char update_func_sub_code){
    int status = -1;
    status = uart_config();
    if(status == -1){
        printf("Error configuring UART\n");
        return status;
    }
    func_code = update_func_code;
    func_sub_code = update_func_sub_code;
    return status;
}

void modbus_close(){
    uart_close();
}

int modbus_receive(unsigned char *message, unsigned char size) {
    int len = 0;
    int i;
    for (i = 0; i < TIMEOUT; i++) {
        usleep(SLEEP_TIME * 1000);
        len = uart_read(message, size);
        if (len != 0) break;
    }

    if (len <= 0) return len;

    unsigned short crc;
    int len_crc = len - sizeof(unsigned short);
    memcpy(&crc, &message[len_crc], sizeof(unsigned short));
    if(crc != calcula_CRC(message, len_crc)){
        printf("CRC error\n");
        return -1;
    }
    return len;
}

int modbus_send(unsigned char *message, unsigned char size) {
    unsigned char *buffer = malloc(255);
    if (buffer == NULL) {
        printf("Error allocating memory\n");
        return -1;
    }

    int count = 0;
    memcpy(&buffer[count++], &device_code, sizeof(unsigned char));
    memcpy(&buffer[count++], &func_code, sizeof(unsigned char));
    memcpy(&buffer[count++], &func_sub_code, sizeof(unsigned char));

    memcpy(&buffer[count], matricula, sizeof(matricula));
    count += sizeof(matricula);

    memcpy(&buffer[count], message, size);
    count += size;

    unsigned short crc = calcula_CRC(buffer, count);
    memcpy(&buffer[count], &crc, sizeof(unsigned short));
    count += sizeof(unsigned short);

    int final_size = uart_write(buffer,count);
    free(buffer);
    return final_size;
}