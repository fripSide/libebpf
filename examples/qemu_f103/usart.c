#include "usart.h"
#include "reg.h"
#include "stdint.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define __NOIST __attribute__((no_instrument_function))

static bool is_init = false;

__NOIST void
usart_init() 
{
    *(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
    *(RCC_APB1ENR) |= (uint32_t) (0x00020000);

    /* USART2 Configuration, Rx->PA3, Tx->PA2 */
    *(GPIOA_CRL) = 0x00004B00;
    *(GPIOA_CRH) = 0x44444444;
    *(GPIOA_ODR) = 0x00000000;
    *(GPIOA_BSRR) = 0x00000000;
    *(GPIOA_BRR) = 0x00000000;

    *(USART2_CR1) = 0x0000000C;
    *(USART2_CR2) = 0x00000000;
    *(USART2_CR3) = 0x00000000;
    *(USART2_CR1) |= 0x2000;
    is_init = true;
}

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)

__NOIST void
print_str(const char *str)
{
    if (!is_init) return;
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

__NOIST void uart_putchar(char c) {
    while (!(*(USART2_SR) & USART_FLAG_TXE));
	*(USART2_DR) = (c & 0xFF);
}

__NOIST char uart_getchar() {
	while (!(*(USART2_SR) & USART_FLAG_RXNE));
	return (*USART2_DR & (uint16_t) 0x01FF);
}


__NOIST void my_printf(const char *format, ...) {
	va_list ap;

	char str[128] = {0};

	va_start(ap, format);
	vsnprintf(str, 128, format, ap);
	va_end(ap);
	print_str(str);
}
