#ifndef _COMMAND_H
#define _COMMAND_H 
void input_buffer_overflow_message(char *str);
void command_help();
void command_hello();
void command_reboot();
void command_cancel_reboot();
void command_not_found();
void command_ls();
void command_cat(char *str);
void command_test_alloc();
#endif

