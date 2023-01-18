#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "csv.h"

int get_csv_data(int * time_arr, float * temp_arr) {
    FILE *file_ptr = fopen("curva_reflow.csv", "r");
    if (file_ptr == NULL) {
        fprintf(stderr, "Error opening file\n");
        return -1;
    }

    char line[1000];
    char *data;
    int line_count = 0;

    // read the file line by line
    while (fgets(line, sizeof(line), file_ptr) != NULL) {
        data = strtok(line, ",");
        
        time_arr[line_count] = atoi(data);

        data = strtok(NULL, ",");

        temp_arr[line_count] = atof(data);
        line_count++;
    }

    fclose(file_ptr);
    return line_count;
}

void append_csv_data(float internal_temp, float external_temp, float temp_pot, float power_cooler, float power_resistor) {
    FILE *file_ptr = fopen("logs.csv", "a");

    if (file_ptr == NULL) {
        fprintf(stderr, "Error opening the logs file\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* time_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    fprintf(file_ptr, "%s, %f, %f, %f, %f, %f\n",time_str , internal_temp, external_temp, temp_pot, power_cooler, power_resistor);

    fclose(file_ptr);
}