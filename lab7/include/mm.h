#ifndef	_MM_H
#define	_MM_H

#define PAGE_SIZE 4096

#define NULL 0

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned long n);

#endif

#endif  /*_MM_H */
