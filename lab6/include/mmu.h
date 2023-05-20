#ifndef _MMU_H
#define _MMU_H

// Kernel virture base
#define KERNEL_VIRTURE_BASE       0xFFFF000000000000
#define PAGE_MASK                ~0xFFF

#define PHYS_TO_VIRT(x) (x + 0xffff000000000000)
#define VIRT_TO_PHYS(x) (x - 0xffff000000000000)


// SCTLR_EL1, System Control Register (EL1)

#define SCTLR_VALUE_MMU_DISABLED 0
#define SCTLR_VALUE_MMU_ENABLE 1

// TCR_EL1, Translation Control Register

#define TTBR0_EL1_REGION_BIT 48
// The size offset of the memory region addressed by TTBR0_EL1.
// The region size is 2^(64-T0SZ) bytes

#define TTBR1_EL1_REGION_BIT 48
// The size offset of the memory region addressed by TTBR1_EL1.
// The region size is 2^(64-T1SZ) bytes


#define TTBR0_EL1_GRANULE    0b00
// Granule size for the TTBR0_EL1
//       TG0            Meaning
//       0b00             4KB
//       0b01            64KB
//       0b10            16KB

#define TTBR1_EL1_GRANULE    0b10
// Granule size for the TTBR1_EL1
//       TG1            Meaning
//       0b01            16KB
//       0b10             4KB
//       0b11            64KB

#define TCR_EL1_T0SZ         ((64 - TTBR0_EL1_REGION_BIT) << 0)
// T0SZ, bits[5:0]
#define TCR_EL1_T1SZ         ((64 - TTBR1_EL1_REGION_BIT) << 16)
// T1SZ, bits[21:16]
#define TCR_EL1_TG0          (TTBR0_EL1_GRANULE << 14)
// TG0, bits[15:14]
#define TCR_EL1_TG1          (TTBR1_EL1_GRANULE << 30)
// TG1, bits[31:30]

#define TCR_EL1_VALUE        (TCR_EL1_T0SZ | TCR_EL1_T1SZ | TCR_EL1_TG0 | TCR_EL1_TG1)


// MAIR_EL1  Memory Attribute Indirection Register (MAIR)

// Lab spec
// Device memory nGnRnE:
//	Peripheral access
//	The most restricted memory access
// Normal memory without cache:
//	Normal RAM access
//	Memory gathering, reordering, and speculative excution are possible but without cache

/*
 * Memory region attributes:
 *
 *   n = AttrIndx[2:0]
 *			n	MAIR
 *   DEVICE_nGnRnE	000	00000000
 *   NORMAL_NC		001	01000100
 */

#define MAIR_EL1_DEVICE_nGnRnE       0b00000000
#define MAIR_EL1_NORMAL_NOCACHE      0b01000100
#define MAIR_EL1_IDX_DEVICE_nGnRnE 0
#define MAIR_EL1_IDX_NORMAL_NOCACHE 1
#define MAIR_EL1_VALUE ((MAIR_EL1_DEVICE_nGnRnE << (MAIR_EL1_IDX_DEVICE_nGnRnE * 8)) |(MAIR_EL1_NORMAL_NOCACHE << (MAIR_EL1_IDX_NORMAL_NOCACHE * 8)))


// Page Descriptor (PD)
/*
                           Descriptor format
`+------------------------------------------------------------------------------------------+
 | Upper attributes | Address (bits 47:12) | Lower attributes | Block/table bit | Valid bit |
 +------------------------------------------------------------------------------------------+
 63                 47                     11                 2                 1           0

Bits[1:0] Specify the next level is a block/page, page table, or invalid.
Bits[4:2] The index to MAIR.
Bits[6] 0 for only kernel access, 1 for user/kernel access.
Bits[7] 0 for read-write, 1 for read-only.
Bits[10] The access flag, a page fault is generated if not set.
Bits[47:n] The physical address the entry point to. Note that the address should be aligned to Byte.
Bits[53] The privileged execute-never bit, non-executable page frame for EL1 if set.
Bits[54] The unprivileged execute-never bit, non-executable page frame for EL0 if set.
*/

#define PD_TABLE         0b11
#define PD_BLOCK         0b01
#define PD_PAGE          0b11
#define PD_ACCESS        (1 << 10)
#define ACCESS_PERM_RW   (0b01 << 6)
#define PD_MASK          0x1FFUL

#define PGD0_ATTR        PD_TABLE

#define PUD0_ATTR        PD_TABLE // for normal-memory (*)
#define PUD1_ATTR        (PD_ACCESS | (MAIR_EL1_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK) // for device-memory (*)
#define PMD0_ATTR        PD_TABLE 

#define PTE_DEVICE_ATTR  (PD_ACCESS | (MAIR_EL1_IDX_DEVICE_nGnRnE << 2) | PD_PAGE) // for deveice-memory
#define PTE_NORAL_ATTR   (PD_ACCESS | (MAIR_EL1_IDX_NORMAL_NOCACHE << 2) | PD_PAGE) // for normal-memory

#define PGD_SHIFT      39
#define PUD_SHIFT      30
#define PMD_SHIFT      21
#define PTE_SHIFT      12

#endif
