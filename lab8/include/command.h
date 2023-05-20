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

void command_fdt_traverse();

void command_lab3_basic_1(char *str);

void command_settimeout(char *str);

void command_test_buddy_print();

void command_test_dynamic_alloc_print();

#endif

