#pragma once

void usart_init();
void uart_putchar(char c);
char uart_getchar();
void print_str(const char *str);
void my_printf(const char *format, ...);