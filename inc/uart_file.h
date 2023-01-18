#ifndef UART_FILE_
#define UART_FILE_

void uart_close();
int uart_config();
int uart_read(unsigned char* message, unsigned int size);
int uart_write(unsigned char* buffer, unsigned int index);

#endif
