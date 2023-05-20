#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_send( char c );

char uart_recv( void );

void uart_puts_width(unsigned char* str, int width);

void uart_send_string(char* str);

void uart_send_string_int2hex(int value);

void uart_send_string_longlong2hex(long long value);

void uart_print_long(long long l);

void uart_print_int(int i);

void uart_print_uint(unsigned int i);

//void uart_print_uint32_t(uint32_t i);

void uart_printf(char *fmt, ...);

void uart_init ( void );

#endif  /*_MINI_UART_H */
