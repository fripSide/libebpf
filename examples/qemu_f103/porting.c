#ifdef DEV_QEMU
#include "usart.h"
#include <stdarg.h>

char shell_get_char() {
	return uart_getchar();
}

void shell_put_char(char c) {
	uart_putchar(c);
}

void log_print_porting(const char *fmt, va_list *args) {
	char str[128] = {0};
	vsnprintf(str, 128, fmt, *args);
	print_str(str);
}

#endif