#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#include "inc/modbus_functions.h"
#include "inc/bme280_driver.h"
#include "inc/temperature_controller.h"
#include "inc/pid.h"
#include "inc/csv.h"

#define DEFAULT_KP 30.0
#define DEFAULT_KI 0.2
#define DEFAULT_KD 400.0
#define TEMPERATURE_MIN 0
#define TEMPERATURE_MAX 100

void final_config(){
    printf("Finalizing program\n");
    close_temp();
    send_int_control(0);
    send_functioning_state(0);
    send_system_state(0);
    exit(0);
}

void temperature_control_loop(float temp_pot, int mode);
void curve_mode();

int current_mode = 0;
float debug_temperature;

void temperature_control_loop(float temp_pot, int mode) {
    int command = 0;
    float prev_temp = 30;

    // print mode 
    if(mode == 0){
        printf(" ---- Debug --- \n");
        send_system_mode(1);
    } else {
        send_system_mode(0);
        printf(" ---- Manual --- \n");
    }

    int count = 0;

    // send send_ambient_temperature at each 2 iterations
    while(command == 0 || command == 1 || command == -1 || command == 3) {
        // sleep for 0.5 seconds
        usleep(500000);

        if (mode == 1) { // manual mode
            temp_pot = get_potentiometer_temperature();
            if (temp_pot <= 0) {
                printf("Error getting potentiometer temperature\n");
                continue;
            }
        }

        double external_temperature;
        if(get_temp(&external_temperature) != 0) {
            fprintf(stderr, "Error getting external temperature\n");
            continue;
        }

        pid_atualiza_referencia(temp_pot);

        float internal_temperature = get_internal_temperature();

        if(internal_temperature <= TEMPERATURE_MIN || internal_temperature > TEMPERATURE_MAX){
            internal_temperature = prev_temp;
        }
        prev_temp = internal_temperature;

        float power = pid_controle(internal_temperature);
        
        send_int_control(power);
        send_float_control(temp_pot);
        
        float power_cooler = 0;
        float power_resistor = 0;
        if(power < 0){
            power_cooler = power * -1;
            if(power_cooler < 40) power_cooler = 40;
        } else {
            power_resistor = power;
        }

        append_csv_data(internal_temperature, external_temperature, temp_pot, power_cooler, power_resistor);
        printf("Internal temperature: %.2f External temperature: %.2f Reference temperature: %.2f\n", internal_temperature, external_temperature, temp_pot);
        
        change_temperature(power);
        command = get_user_command();

        count = (count + 1) % 20;
        if(count == 0){
            send_ambient_temperature(external_temperature);
        }
    }
    if (command == 4) {
        printf(" ---- Waiting for command --- \n");
    }
    while (command == 4 || command == 0) {
        usleep(500000);
        command = get_user_command();
        // print command
        printf("Command: %d\n", command);
    }
    if(command == 2){
        final_config();
    }
    else if(command == 5){ // command: change mode
        // else call curve mode
        if(mode == 0){ // debug mode
            temperature_control_loop(temp_pot, 1);
        } else { // manual mode
            if (current_mode == 1) {
                curve_mode();
            } else {
                temperature_control_loop(debug_temperature, 0);
            }
        }
    } else {
        temperature_control_loop(temp_pot, mode);
    }
}

void curve_mode(){
    printf(" ---- Curva --- \n");
    send_system_mode(1);
    float prev_temp = 30;

    int time[600];
    float temp[600];

    int size = get_csv_data(time, temp);
    // print time and temp
    for(int i = 0; i < size; i++){
        printf("Time: %d Temp: %.2f\n", time[i], temp[i]);
    }

    int count = 0;
    float current_target_temp = temp[1];
    int next_time = time[2];
    int time_index = 2;

    // print values
    printf("Current target temp: %.2f Next time: %d\n", current_target_temp, next_time);

    int command = 0;
    while(command == 0 || command == 1 || command == 3 || command == -1){
        // sleep for 0.5 seconds
        usleep(500000);

        if(count == (next_time)){
            if(time_index == size){
                final_config();
            }
            current_target_temp = temp[time_index];
            next_time = time[time_index+1];

            time_index++;
        }
        double external_temperature;
        if(get_temp(&external_temperature) != 0) {
            fprintf(stderr, "Error getting external temperature\n");
            continue;
        }

        pid_atualiza_referencia(current_target_temp);

        float internal_temperature = get_internal_temperature();

        if(internal_temperature <= TEMPERATURE_MIN || internal_temperature > TEMPERATURE_MAX){
            internal_temperature = prev_temp;
        }
        prev_temp = internal_temperature;

        float power = pid_controle(internal_temperature);

        float power_cooler = 0;
        float power_resistor = 0;
        if(power < 0){
            power_cooler = power * -1;
            if(power_cooler < 40) power_cooler = 40;
        } else {
            power_resistor = power;
        }

        send_int_control(power);
        send_float_control(current_target_temp);

        append_csv_data(internal_temperature, external_temperature, current_target_temp, power_cooler, power_resistor);
        printf("Internal temperature: %.2f External temperature: %.2f Reference temperature: %.2f\n", internal_temperature, external_temperature, current_target_temp);

        change_temperature(power);

        count++;

        command = get_user_command();
    }
    if(command == 2){
        final_config();
    }
    if(command == 5){
        temperature_control_loop(30, 1);
    }
}

int init_pid_values(float *kp, float *ki, float *kd){
    printf("PID constants:\n");
    printf("1. Manually adjust Kp, Ki, and Kd\n");
    printf("2. Use default values\n");

    int user_input;
    scanf("%d", &user_input);

    if (user_input == 1){
        printf("kp: ");
        scanf("%f", kp);

        printf("ki: ");
        scanf("%f", ki);

        printf("kd: ");
        scanf("%f", kd);
    } else if (user_input == 2){
        printf("Using default values\n");
        *kp = DEFAULT_KP;
        *ki = DEFAULT_KI;
        *kd = DEFAULT_KD;
    } else {
        printf("INVALID OPTION!\n");
        return -1;
    }
    return 0;
}

int init_bme280() {
    if(init_bme() != 0) {
        fprintf(stderr, "Error initializing BME280\n");
        return -1;
    }
    return 0;
}

int select_usage_mode() {
    printf("--------------------\n");
    printf("| MODE Menu |\n");
    printf("--------------------\n");
    printf("Usage mode:\n");
    printf("1. Debug mode\n");
    printf("2. Dashboard mode\n");

    int user_input;
    scanf("%d", &user_input);

    if (user_input == 1){
        printf("Debug mode\n");
        return 0;
    } else if (user_input == 2){
        printf("Dashboard mode\n");
        return 1;
    } else {
        printf("INVALID OPTION!\n");
        return -1;
    }
}

int main(){
    signal(SIGINT, final_config);
    send_system_state(0);

    printf("--------------------\n");
    printf("| PID Control Menu |\n");
    printf("--------------------\n");

    float kp, ki, kd;
    if(init_pid_values(&kp, &ki, &kd) != 0) {
        return -1;
    }

    printf("kp: %f, ki: %f, kd: %f\n", kp, ki, kd);

    if(init_bme280() != 0) {
        return -1;
    }
    send_system_state(1);
    pid_configura_constantes(kp,ki,kd);
    config_temperature();

    int usage_mode = select_usage_mode();
    current_mode = usage_mode;
    if (usage_mode == 0){  // debug mode
        printf("Enter desired temperature: ");
        scanf("%f",&debug_temperature);
    }

    int count = 0;
    printf("Waiting for dashboard command\n");
    while(count < 240){
        sleep(1);

        int command = get_user_command();

        switch(command){
            case 1:
                break;
            case 2:
                final_config();
                break;
            case 3:
                temperature_control_loop(debug_temperature, usage_mode);
                break;
        }

        
        count++;
    }

    final_config();
    return 0;
}