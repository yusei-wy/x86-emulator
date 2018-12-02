TARGET = call-test.bin
Z_TOOLS = ../z_tools

CFLAGS += -nostdlib -fno-asynchronous-unwind-tables \
	-I$(Z_TOOLS)/i386-elf-gcc/include -g -fno-stack-protector
LDFLAGS += --entry=func --oformat=binary

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	gcc $(CFLAGS) -c $<

%.bin : %.o Makefile
	ld $(LDFLAGS) -o $@ $<

%.bin : %.asm Makefile
	nasm -f bin -o $@ $<
