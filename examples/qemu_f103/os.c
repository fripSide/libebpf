#include <stdint.h>
#include <stdio.h>
#include "usart.h"
#include <stdarg.h>
#include <stdbool.h>
// #include "jit.h"

#define __NAKE __attribute__((naked))
#define __NOIST __attribute__((no_instrument_function))

// void __attribute__((no_instrument_function, naked)) 
// restore_context() {
// 	__asm__ __volatile__("BX lr");
// }

void __attribute__((no_instrument_function))
_patch_dispatch(uint32_t *ctx) {
	my_printf("_patch_dispatch: %p\n", ctx);
	// restore_context();
	print_str("after _patch_dispatch\n");
}

void __attribute__((no_instrument_function, naked))
__cyg_profile_func_enter(void *this_func, void *call_site) {
	//__asm("push {r0-r3, r12, lr}");
	//print_str("enter func\n");
	__asm__ __volatile__("PUSH {lr}");
	__asm__ __volatile__("TST lr, #4");
	__asm__ __volatile__("ITE EQ"); 
	__asm__ __volatile__("MRSEQ r0, MSP"); 
	__asm__ __volatile__("MRSNE r0, PSP"); 
	__asm__ __volatile__("B _patch_dispatch"); 
	__asm__ __volatile__("POP {lr}");
	__asm__ __volatile__("BX lr");
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_fn, void *call_site) {
	print_str("exit func\n");
}

bool my_func_test(int a, int b, int c) {
	int t = a + b + c;
	if (t > 2500) {
		return false;
	}

	return true;
}

#include "ihp_cli.h"
#include "hotpatch/include/ebpf_test.h"

void main(void)
{
	usart_init();
	// bool res = my_func_test(500, 1000, 2000);
	// print_str("Is bug fixed? ");
	// print_str(res ? "yes": "no");
	print_str("Start Qemu Test\n");
	// jit_run_test();

	// typedef bool (*myfunc) (int a, int b, int c);
	// myfunc func = my_func_test;
	// func(1, 2, 3);

	// run_shell_cli();
	//run_test_by_id(3);
	// float t = 1.5;
	// my_printf("%f\n", t);
	// 3 6
	run_ebpf_eva(3);

	//nrf_conn(pkt);

	while (true) {
		char c = uart_getchar();
		char str[5] = {0};
		str[0] = c;
		my_printf("char uart_get_char(): %s %d\n", str, c);
		if (c == 'q') {
			break;
		}
	}
	print_str("Looping...\n");
	while (1);
}
