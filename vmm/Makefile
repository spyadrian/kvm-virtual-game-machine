CC=gcc -std=gnu17 -Wall -Wextra -MMD -Ishared -I../.. -I..
LIBS=-lSDL2 -lSDL2_image

C_SRCS=$(shell find . -name "*.c")
C_OBJS=$(C_SRCS:.c=.o)
C_DEPS=$(C_OBJS:%.o=%.d)

$(OUT): $(C_OBJS)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@

create_disk:
	qemu-img create -f raw $(DISK) $(DISK_SIZE)

clean:
	rm -f $(C_OBJS) $(C_DEPS) *.o *.d $(OUT) $(DISK)

.PHONY: clean

-include $(C_DEPS)
