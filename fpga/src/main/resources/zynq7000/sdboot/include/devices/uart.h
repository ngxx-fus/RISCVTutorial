// See LICENSE.Sifive for license details.

/*
 * SiFive UART IP Register Offsets and Bitmasks
 * This header defines the relative register map and control bits for the 
 * SiFive UART controller, typically instantiated inside the RISC-V Rocket Chip PL.
 *
 * NOTE:
 * - These are RELATIVE offsets. To access physical registers, they must be added 
 *   to the UART Base Address in your design (typically 0x10013000 or similar).
 * - The baud rate divisor register (UART_REG_DIV) is strictly clock-dependent. 
 *   When porting from Arty 35T to Zynq-7000, the PL clock frequency (FCLK_CLKx) 
 *   might change. You MUST update the system clock constant in your software 
 *   to calculate the correct divisor value, otherwise, serial output will be corrupted.
 *
 * WARN:
 * - DO NOT use this header or these offsets for the Zynq-7000 hard PS UART controllers 
 *   (UART0 at 0xE000_0000 or UART1 at 0xE000_1000). 
 * - The hard Zynq PS UART uses a completely different Cadence-based architecture 
 *   and register layout (e.g., Control Register at 0x00, Mode Register at 0x04, 
 *   and Baud Rate Generator at 0x18). Using these SiFive offsets on PS UART memory 
 *   will lead to bus faults or unpredictable behavior.
 */

#ifndef _SIFIVE_UART_H
#define _SIFIVE_UART_H

/* Register offsets */
#define UART_REG_TXFIFO         0x00
#define UART_REG_RXFIFO         0x04
#define UART_REG_TXCTRL         0x08
#define UART_REG_RXCTRL         0x0c
#define UART_REG_IE             0x10
#define UART_REG_IP             0x14
#define UART_REG_DIV            0x18

/* TXCTRL register */
#define UART_TXEN               0x1
#define UART_TXNSTOP            0x2
#define UART_TXWM(x)            (((x) & 0xffff) << 16)

/* RXCTRL register */
#define UART_RXEN               0x1
#define UART_RXWM(x)            (((x) & 0xffff) << 16)

/* IP register */
#define UART_IP_TXWM            0x1
#define UART_IP_RXWM            0x2

#endif /* _SIFIVE_UART_H */
