/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART declarations
 *
 * $Id: uart.h,v 1.1.2.1 2005/12/28 22:35:08 joerg_wunsch Exp $
 */

/*
 * Perform UART startup initialization.
 */
void uart0_init(void);
void uart1_init(void);

/*
 * Send one character to the UART.
 */
int uart0_putchar(char c, FILE *stream);
int uart1_putchar(char c, FILE *stream);

/*
 * Size of internal line buffer used by uart_getchar().
 */
#define RX0_BUFSIZE 40
#define RX1_BUFSIZE 40

/*
 * Receive one character from the UART.  The actual reception is
 * line-buffered, and one character is returned from the buffer at
 * each invokation.
 */
int	uart0_getchar(FILE *stream);
int	uart1_getchar(FILE *stream);
