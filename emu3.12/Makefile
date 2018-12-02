TARGET = px86
OBJS = main.o emulator_function.o instruction.o modrm.o io.o
Z_TOOLS = ./z_tools

CFLAGS += -Wall

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	gcc $(CFLAGS) -c $<

$(TARGET) : $(OBJS) Makefile
	gcc -o $@ $(OBJS)

clean : Makefile
	rm *.o
