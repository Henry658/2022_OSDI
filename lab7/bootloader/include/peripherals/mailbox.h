#ifndef       _P_MAILBOX_H
#define       _P_MAILBOX_H

#include "peripherals/base.h"

#define MAILBOX_BASE PBASE + 0xb880


#define MAILBOX_READ    (unsigned int*)(MAILBOX_BASE)
#define MAILBOX_STATUS  (unsigned int*)(MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (unsigned int*)(MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY                     0x40000000
#define MAILBOX_FULL                      0x80000000

#define MAILBOX_CODE_BUF_REQ              0x00000000
#define MAILBOX_CODE_BUF_RES_SUCC         0x80000000
#define MAILBOX_CODE_TAG_REQ              0x00000000

#define MAILBOX_TAG_GET_BOARD_REVISION    0x00010002
#define MAILBOX_TAG_GET_VC_MEMORY         0x00010006
#define MAILBOX_TAG_SET_CLOCK_RATE        0x00038002
#define MAILBOX_TAG_SET_PHY_WIDTH_HEIGHT  0x00048003
#define MAILBOX_TAG_SET_VTL_WIDTH_HEIGHT  0x00048004
#define MAILBOX_TAG_SET_VTL_OFFSET        0x00048009
#define MAILBOX_TAG_SET_DEPTH             0x00048005
#define MAILBOX_TAG_SET_PIXEL_ORDER       0x00048006
#define MAILBOX_TAG_ALLOCATE_BUFFER       0x00040001
#define MAILBOX_TAG_GET_PITCH             0x00040008

#endif
 
