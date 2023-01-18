#ifndef MODBUS_FUNCTIONS_H
#define MODBUS_FUNCTIONS_H

float get_internal_temperature();
float get_potentiometer_temperature();
int get_user_command();
void send_int_control(int control_signal);
void send_float_control(float control_signal);
void send_system_state(int control_signal);
void send_system_mode(int control_signal);
void send_functioning_state(int control_signal);
void send_ambient_temperature(float temperature);

#endif