TARGET  := riscv64-unknown-elf
CC      := $(TARGET)-gcc
OBJDUMP := $(TARGET)-objdump

# Target 32-bit RISC-V Rocket Core
ARCH    := rv32imac
ABI     := ilp32

CFLAGS  := -march=$(ARCH) -mabi=$(ABI) -mcmodel=medany -std=gnu99 -O2 \
           -fno-common -fno-builtin-printf -Wall -fno-tree-loop-distribute-patterns
LDFLAGS := -static -nostdlib -nostartfiles -T link.ld

PROGRAMS := ethcomm_test

all: $(addsuffix .riscv,$(PROGRAMS)) dumps

%.o: %.c mmio.h util.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__=1 -c $< -o $@

%.riscv: %.o crt.o syscalls.o link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $< crt.o syscalls.o -o $@

dumps: $(addsuffix .dump,$(PROGRAMS))

%.dump: %.riscv
	$(OBJDUMP) -d $< > $@

clean:
	rm -f *.o *.riscv *.dump

.PHONY: all dumps clean