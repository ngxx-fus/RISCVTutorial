// See LICENSE.Sifive for license details.

/*
 * SiFive PLIC (Platform-Level Interrupt Controller) Register Offsets
 * This header defines the relative address offsets and bit shift parameters
 * for the RISC-V PLIC, which handles and prioritizes external interrupts 
 * for the Rocket Chip CPU core(s).
 *
 * NOTE:
 * - These are RELATIVE offsets. To access the physical registers, they must be 
 *   added to the PLIC Base Address configured in your Chisel generator 
 *   (typically 0x0C00_0000 in default Rocket Chip configurations).
 * - Ensure that the interrupt Source IDs mapped to PLIC priority/pending registers 
 *   match the exact hardware interrupt connection indexes in your Vivado Block Design.
 *
 * WARN:
 * - This RISC-V PLIC is completely distinct from the Zynq-7000 ARM Cortex-A9's 
 *   hard GIC (Generic Interrupt Controller). Do not mix up their registers or drivers.
 * - If you want the RISC-V core to handle interrupts from Zynq PS peripherals, 
 *   you must explicitly route those interrupts from the PS to the PL fabric in Vivado 
 *   and tie them to the Rocket Chip external interrupt lines.
 */

#ifndef PLIC_H
#define PLIC_H

#include <const.h>

// 32 bits per source
#define PLIC_PRIORITY_OFFSET            _AC(0x0000,UL)
#define PLIC_PRIORITY_SHIFT_PER_SOURCE  2
// 1 bit per source (1 address)
#define PLIC_PENDING_OFFSET             _AC(0x1000,UL)
#define PLIC_PENDING_SHIFT_PER_SOURCE   0

//0x80 per target
#define PLIC_ENABLE_OFFSET              _AC(0x2000,UL)
#define PLIC_ENABLE_SHIFT_PER_TARGET    7


#define PLIC_THRESHOLD_OFFSET           _AC(0x200000,UL)
#define PLIC_CLAIM_OFFSET               _AC(0x200004,UL)
#define PLIC_THRESHOLD_SHIFT_PER_TARGET 12
#define PLIC_CLAIM_SHIFT_PER_TARGET     12

#define PLIC_MAX_SOURCE                 1023
#define PLIC_SOURCE_MASK                0x3FF

#define PLIC_MAX_TARGET                 15871
#define PLIC_TARGET_MASK                0x3FFF

#endif /* PLIC_H */
