#include <stdio.h>
#include <stddef.h>

#include <kernel/tty.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Welcome to Olympos!\n");
	printf("An experimental 32-bit Operating System\n");

	printf("\nTest printf function with different argument types:\n");

	char letter = 'F';
	printf("Character: %c\n", letter);

	const char *str = "Hello, World!\n";
	printf("String: %s", str);

	int a = 27;
	printf("Integer: %d\n", a);
	printf("Negative Integer: %d\n", -42);

	int x = 10;
	int *ptr = &x;
	printf("Address of x: %p\n", (void*)ptr);
	printf("Address of ptr: %p\n", (void*)&ptr);

	// Terminal Scroll Test
	for (size_t i = 0; i < 40; i++) {
		printf("Line: %d\n", i);
	}
}
