#ifndef CSV_
#define CSV_

int get_csv_data(int * time, float * temp);
void append_csv_data(float internal_temp, float external_temp, float temp_pot, float power_cooler, float power_resistor);

#endif
