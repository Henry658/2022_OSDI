
extern int __kernel_size;

void * freeptr = 0x80000 + &__kernel_size; 

void * simple_alloc(unsigned int size){
    freeptr += size;
    return freeptr - size;

}
