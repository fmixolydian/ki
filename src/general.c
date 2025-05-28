#include "ki.h"

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