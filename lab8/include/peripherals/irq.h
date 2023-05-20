#include "peripherals/base.h"

#ifndef _IRQ_H
#define _IRQ_H

#define IRQ_BASE             (PBASE + 0xb000)

#define IRQ_BASIC_PENDING    (IRQ_BASE + 0x200)
#define IRQ_PENDING_1        (IRQ_BASE + 0x204)
#define IRQ_PENDING_2        (IRQ_BASE + 0x208)
#define FIQ_CONTROL          (IRQ_BASE + 0x20C)
#define ENABLE_IRQS_1        (IRQ_BASE + 0x210)
#define ENABLE_IRQS_2        (IRQ_BASE + 0x214)
#define ENABLE_BASIC_IRQS    (IRQ_BASE + 0x218)
#define DISABLE_IRQS_1       (IRQ_BASE + 0x21C)
#define DISABLE_IRQS_2       (IRQ_BASE + 0x220)
#define DISABLE_BASIC_IRQS   (IRQ_BASE + 0x224)

#define SYSTEM_TIMER_IRQ_0   (1 << 0)
#define SYSTEM_TIMER_IRQ_1   (1 << 1)
#define SYSTEM_TIMER_IRQ_2   (1 << 2)
#define SYSTEM_TIMER_IRQ_3   (1 << 3)

#endif
