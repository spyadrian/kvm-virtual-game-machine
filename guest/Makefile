BAREMETAL_FLAGS=-m32 -march=i386 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pie -static
CC=gcc -std=gnu17 $(BAREMETAL_FLAGS) -Wall -Wextra -MMD -Ishared -I../.. -I..
CC+=-DPV=$(PV)

LD=gcc -Tguest.ld $(BAREMETAL_FLAGS)

C_SRCS=$(shell find . -name "*.c")
C_OBJS=$(C_SRCS:.c=.o)
C_DEPS=$(C_OBJS:%.o=%.d)

ASM_SRCS=$(shell find . -name "*.s")
ASM_OBJS=$(ASM_SRCS:.s=.o)

$(OUT): $(C_OBJS) $(ASM_OBJS)
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

%.o: %.s
	nasm -f elf32 $< -o $@

clean:
	rm -f $(C_OBJS) $(ASM_OBJS) $(C_DEPS) *.o *.d $(OUT)

.PHONY: clean

-include $(C_DEPS)
