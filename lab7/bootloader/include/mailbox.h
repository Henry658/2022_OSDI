#ifndef _MAILBOX_H
#define _MAILBOX_H

int mailbox_call(unsigned int *mailbox, unsigned char channel);
void mailbox_board_revision();
void mailbox_vc_memory();

#endif
