// See LICENSE.Sifive for license details.

/*
 * SiFive GPIO IP Register Offsets
 * This header defines the relative address offsets for the SiFive GPIO controller.
 * It is used for bare-metal firmware running on the RISC-V Rocket Chip.
 *
 * NOTE:
 * - These are RELATIVE offsets. To access physical registers, they must be added 
 *   to the GPIO Base Address configured in your Chisel/Rocket Chip generator 
 *   (typically 0x10012000 or similar).
 * - When porting from Arty 35T to Zynq-7000, ensure that the physical FPGA pin 
 *   constraints (.xdc file) in Vivado are updated to map these soft PL GPIO signals 
 *   to the correct Zynq-7000 SelectIO package pins.
 *
 * WARN:
 * - DO NOT use these offsets to control the Zynq-7000 hard Processing System (PS) 
 *   GPIO controller (XGPIOPS, Base Address 0xE000_A000).
 * - The hard Zynq PS GPIO has a completely different register architecture 
 *   (using DIRM at 0x204, OEN at 0x208, and DATA at 0x040). Applying these SiFive 
 *   offsets to the Zynq PS memory space will cause register corruption, undefined 
 *   hardware behavior, or AXI bus locks.
 */

#ifndef _SIFIVE_GPIO_H
#define _SIFIVE_GPIO_H

#define GPIO_INPUT_VAL  (0x00)
#define GPIO_INPUT_EN   (0x04)
#define GPIO_OUTPUT_EN  (0x08)
#define GPIO_OUTPUT_VAL (0x0C)
#define GPIO_PULLUP_EN  (0x10)
#define GPIO_DRIVE      (0x14)
#define GPIO_RISE_IE    (0x18)
#define GPIO_RISE_IP    (0x1C)
#define GPIO_FALL_IE    (0x20)
#define GPIO_FALL_IP    (0x24)
#define GPIO_HIGH_IE    (0x28)
#define GPIO_HIGH_IP    (0x2C)
#define GPIO_LOW_IE     (0x30)
#define GPIO_LOW_IP     (0x34)
#define GPIO_IOF_EN     (0x38)
#define GPIO_IOF_SEL    (0x3C)
#define GPIO_OUTPUT_XOR    (0x40)

#endif /* _SIFIVE_GPIO_H */
