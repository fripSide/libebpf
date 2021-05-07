#include "malloc.h"
#include <stddef.h>
#include <string.h>

#define MAX_HEAPS 4096

typedef long Align;

typedef union Header {
    struct {
		union Header *next;
		unsigned int size;
	} s;
    Align x;
} Header;

static unsigned char heaps[MAX_HEAPS];
static unsigned char *program_break = heaps;

// use circle link list as free list
static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */

static void *sbrk(unsigned int nbytes)
{
	if (program_break + nbytes >= heaps
	    && program_break + nbytes < heaps + MAX_HEAPS) {
		unsigned char *previous_pb = program_break;
		program_break += nbytes;
		return (void *) previous_pb;
	}
	return (void *) -1;
}

void *realloc(void* ptr, size_t size) {
	void* np = malloc(size);
	memcpy(np, ptr, size);
	free(ptr);
	return np;
}

void *malloc(unsigned int nbytes) {
	Header *p, *prevp;
	unsigned int nunits;
	void *cp;
	// round(nbytes, header)
	// alloc header->buffer
	nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

	// char log[50] = {0};
	// char *c = "malloc-test ";
	// int pos = sprint_str(log, c, 0);
	// pos = sprint_int(log, sizeof(Header), pos);
	// pos = sprint_str(log, " ", pos);
	// pos = sprint_int(log, nunits, pos);
	// pos = sprint_str(log, "\n", pos);
	// print_str(log);
	
	if (freep == NULL) {
		base.s.next = freep = &base;
		base.s.size = 0;
	}
	prevp = freep;

	for (p = prevp->s.next; ;prevp = p, p = prevp->s.next) {

		if (p->s.size >= nunits) {
			if (p->s.size == nunits) {
				prevp->s.next = p->s.next;
			} else {
				size_t sz = p->s.size - nunits;
				p->s.size = sz;
				p += sz;
				p->s.size = nunits;
			} 

			freep = prevp;
			return (void *)(p + 1);
		}

		// search whole list
		if (p == freep) {
			cp = sbrk(nunits * sizeof(Header));
			if (cp == (void *) -1) {
				return NULL;
			} else {
				p = (Header *) cp;
				p->s.size = nunits;
				free((void *)(p + 1));
				p = freep;
			}
		}
	}
}

void free(void *ap) {
	Header *bp, *p;
	bp = (Header *) ap - 1;

	// p-> bp -> p->s.next
	for (p = freep; ;p = p->s.next) {
		if (bp > p && bp < p->s.next) {
			break;
		}
		if (p >= p->s.next && (bp > p || bp < p->s.next)) {
			break;
		}
	}
	if (bp + bp->s.size == p->s.next) {
		bp->s.size += p->s.next->s.size;
		bp->s.next = p->s.next->s.next;
		// p->s.next = bp;
	} else {
		bp->s.next = p->s.next;
	}
	if (p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.next = bp->s.next;
	} else {
		p->s.next = bp;
	}

	freep = p;
}

void* calloc(size_t nelem, size_t elmsize) {
	size_t sz = nelem * elmsize;
	void *mem = malloc(sz);
	memset(mem, 0, sz);
	return mem;
}