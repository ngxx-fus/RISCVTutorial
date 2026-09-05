// See LICENSE.Sifive for license details.

/*
 * RISC-V Bit Manipulation and Architecture-Specific Utility Macros
 * This header provides generic mathematical utilities (MAX, MIN, CLAMP, ROUND) 
 * and RISC-V instruction/register size abstractions based on the target XLEN.
 *
 * NOTE:
 * - The generic math and bitfield extraction macros (likely, ROUND, MAX, MIN, 
 *   CLAMP, EXTRACT_FIELD, INSERT_FIELD) are highly portable and can run 
 *   on both ARM Cortex-A9 (Zynq PS) and RISC-V (Rocket Chip PL).
 * - For Rocket Chip (PL), ensure your RISC-V GCC toolchain correctly defines 
 *   '__riscv_xlen' (32 or 64) via compiler flags (e.g., -march=rv64 vs -march=rv32) 
 *   to match your Chisel hardware configuration.
 *
 * WARN:
 * - DO NOT compile or use the architecture-specific macros (SLL32, STORE, LOAD, 
 *   LWU, REGBYTES) when writing bare-metal code for the Zynq-7000 ARM Cortex-A9 PS. 
 *   The ARM toolchain does not define '__riscv_xlen', and RISC-V assembly instructions 
 *   (like 'sd', 'ld', 'sllw') are completely incompatible with the ARMv7-A ISA.
 */

#ifndef _RISCV_BITS_H
#define _RISCV_BITS_H

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define ROUNDUP(a, b) ((((a)-1)/(b)+1)*(b))
#define ROUNDDOWN(a, b) ((a)/(b)*(b))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, lo, hi) MIN(MAX(a, lo), hi)

#define EXTRACT_FIELD(val, which) (((val) & (which)) / ((which) & ~((which)-1)))
#define INSERT_FIELD(val, which, fieldval) (((val) & ~(which)) | ((fieldval) * ((which) & ~((which)-1))))

#define STR(x) XSTR(x)
#define XSTR(x) #x

#if __riscv_xlen == 64
# define SLL32    sllw
# define STORE    sd
# define LOAD     ld
# define LWU      lwu
# define LOG_REGBYTES 3
#else
# define SLL32    sll
# define STORE    sw
# define LOAD     lw
# define LWU      lw
# define LOG_REGBYTES 2
#endif
#define REGBYTES (1 << LOG_REGBYTES)

#endif
