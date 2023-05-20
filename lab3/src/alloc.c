extern char _end;
static char * top = & _end;

void * simple_alloc(unsigned int size){
	char *r = top+0x10;
	if(size<0x18)size=0x18;
	size = size + 0x7;
	size = 0x10 + size - size % 0x10;
	*(unsigned int*)(r-0x8) = size;
	top += size;
	return r;
}



/*extern int __kernel_size;

void * freeptr = 0x80000 + &__kernel_size;

void * simple_alloc(unsigned int size){
    freeptr += size;
    return freeptr - size;

}*/
//Todo
void free(void * ptr){
;
}
