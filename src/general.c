#include "ki.h"
#include <stdarg.h>

int ki_strtok(char *dest, size_t len, char *src, char *delim) {
	int i;
	for (i=0; src[i] && i < (len - 1); i++) {
		if (strchr(delim, src[i])) break;
		dest[i] = src[i];
	}
	dest[i] = 0;
	
	return i;
}

void ki_strrev(char *dest, char *src, size_t n) {
	for (int i=0; i<n; i++) {
		dest[i] = src[(n - 1) - i];
	}
}

void *ki_memdup(void *src, size_t n) {
	void *P = calloc(1, n);
	memcpy(P, src, n);
	return P;
}

uint16_t ki_prg_read16(void *P) {
	return *(uint16_t*)P;
}

uint32_t ki_prg_read32(void *P) {
	return *(uint32_t*)P;
}

float ki_prg_readf32(void *P) {
	return *(float*)P;
}

void ki_prg_write16(void *P, uint16_t n) {
	*(uint16_t*)P = n;
}

void ki_prg_write32(void *P, uint32_t n) {
	*(uint32_t*)P = n;
}

void ki_prg_writef32(void *P, float n) {
	*(float*)P = n;
}

size_t ki_compile_writefmt(uint8_t *P, char *format, ...) {
	va_list args;
	size_t offset = 0;
	
	va_start(args, format);
	
	for (int i=0; format[i]; i++) {
		size_t len;
		
		switch (format[i]) {
			case 'o':
				P[offset] = (int8_t) va_arg(args, int);
				offset += 1;
				break;
				
			case 'b':
				P[offset] = (uint8_t) va_arg(args, int);
				offset += 1;
				break;
			case 'w':
				ki_prg_write16(P + offset, (uint16_t) va_arg(args, int));
				offset += 2;
				break;
			case 'd':
				ki_prg_write32(P + offset, (uint32_t) va_arg(args, int));
				offset += 4;
				break;
			case 'f':
				ki_prg_writef32(P + offset, (float) va_arg(args, double));
				offset += 4;
				break;
			case 'B':
				len = va_arg(args, int);
				memcpy(P, va_arg(args, void*), len);
				offset += len;
				break;
		}
		
		printf("FMT %c: %lu\n", format[i], offset);
	}
	
	va_end(args);
	
	printf("FMT %lu\n", offset);
	return offset;
}