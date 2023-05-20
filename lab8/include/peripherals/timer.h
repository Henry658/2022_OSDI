#ifndef _P_TIMER_H
#define _P_TIMER_H

#define KERNEL_VIRTURE_BASE       0xFFFF000000000000

#include "peripherals/base.h"
#define CORE0_TIMER_IRQ_CTRL 0x40000040 + KERNEL_VIRTURE_BASE
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int * )(0x40000060 + KERNEL_VIRTURE_BASE))

#endif
