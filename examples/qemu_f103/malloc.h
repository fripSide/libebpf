#ifndef MALLOC_H_
#define MALLOC_H_
#include <stdint.h>
#include <stddef.h>

void *calloc(size_t nelem, size_t elmsize);
void *realloc(void* ptr, size_t size);
void *malloc(unsigned int nbytes);
void free(void *ap);

#endif