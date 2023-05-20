#include "peripherals/mailbox.h"
#include "string.h"

int mailbox_call(unsigned int *mailbox, unsigned char channel){
    unsigned int r = (unsigned int)(((unsigned long)mailbox) & (~0xF)) | (channel & 0xF);
    
    //wait until full flag unset
    while(*MAILBOX_STATUS & MAILBOX_FULL){
        ; //do nothing
    }
    
    // write address of message + channel to mailbox
    *MAILBOX_WRITE = r;
    
    // wait until response
    while(1){
        // wait until empty flag unset
        while(*MAILBOX_STATUS & MAILBOX_EMPTY){
            ; //do nothing
        }
        if(r == *MAILBOX_READ){
            return mailbox[1] == MAILBOX_CODE_BUF_RES_SUCC;
        }
    }
    return 0;
}
        

void mailbox_board_revision(){
    unsigned int __attribute__((aligned(16))) mailbox[7];
    mailbox[0] = 7*4;
    mailbox[1] = MAILBOX_CODE_BUF_REQ;
    // tags begin    
    mailbox[2] = MAILBOX_TAG_GET_BOARD_REVISION;
    mailbox[3] = 4;
    mailbox[4] = MAILBOX_CODE_TAG_REQ;
    mailbox[5] = 0;
    mailbox[6] = 0x0;
    // tags end
    mailbox_call(mailbox, 8);
    uart_send_string("# Board Revision:    ");
    uart_send_string_int2hex(mailbox[5]);
    uart_send_string("\r\n");
}



void mailbox_vc_memory(){
    unsigned int __attribute__((aligned(16))) mailbox[8];
    mailbox[0] = 8*4;
    mailbox[1] = MAILBOX_CODE_BUF_REQ;
    // tags begin    
    mailbox[2] = MAILBOX_TAG_GET_VC_MEMORY;
    mailbox[3] = 8;
    mailbox[4] = MAILBOX_CODE_TAG_REQ;
    mailbox[5] = 0;
    mailbox[6] = 0;
    mailbox[7] = 0x0;
    // tags end
    mailbox_call(mailbox, 8);
    uart_send_string("# VC Core base address: 0x");
    uart_send_string_int2hex(mailbox[5]);
    uart_send_string(" size: 0x");
    uart_send_string_int2hex(mailbox[6]);
    uart_send_string("\r\n");
    
}
