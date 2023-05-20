#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_puts_width(char * str, int width);
void uart_send_string(char* str);
void uart_send_string_int2hex(char *str);

#endif  /*_MINI_UART_H */
