#ifndef MODBUS_H
#define MODBUS_H

int modbus_config(unsigned char func_code, unsigned char func_sub_code);
void modbus_close();
int modbus_receive(unsigned char *message, unsigned char size);
int modbus_send(unsigned char *message, unsigned char size);

#endif