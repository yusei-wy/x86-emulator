TARGET = test.bin
OBJS = crt0.o test.o
Z_TOOLS = ../z_tools

CC = gcc
AS = nasm
LD = ld
CFLAGS += -m32 -nostdlib -fno-pic -fno-asynchronous-unwind-tables \
	-I$(Z_TOOLS)/i386-elf-gcc/include -g -fno-stack-protector
LDFLAGS += -m elf_i386 --entry=start --oformat=binary -Ttext 0x7c00

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

%.o : %.asm Makefile
	$(AS) -f elf $<

$(TARGET) : $(OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

clean : Makefile
	rm *.o *.bin
