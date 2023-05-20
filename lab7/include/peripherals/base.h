#ifndef	_P_BASE_H
#define	_P_BASE_H

#include "mmu.h"


#define MMIO_PHYSICAL 0x3F000000
#define PBASE         (KERNEL_VIRTURE_BASE | MMIO_PHYSICAL) //MMIO_PHYSICAL 


#endif  /*_P_BASE_H */
