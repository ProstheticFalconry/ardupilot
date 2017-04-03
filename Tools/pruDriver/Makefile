CC = gcc
CFLAGS = -Wall
PRU_ASM = pasm
LDLIBS+= -lpthread -lprussdrv

all: hcsr04 pru_stop pru_start pru_read

hcsr04: hcsr04.p
	@echo "\n>> Generating PRU binary"
	$(PRU_ASM) -b hcsr04.p

pru_stop: pru_stop.c hcsr04.p
	 @echo "\n>> Compiling pru_stop.c"
	$(CC) -c -o pru_stop.o pru_stop.c
	$(CC) -o pru_stop pru_stop.o -lpthread -lprussdrv

pru_start: pru_start.c hcsr04.p
	@echo "\n>> Compiling pru_start.c"
	$(CC) -c -o pru_start.o pru_start.c
	$(CC) -o pru_start pru_start.o -lpthread -lprussdrv

pru_read: pru_read.c hcsr04.p
	@echo "\n>> Compiling pru_read.c"
	$(CC) -c -o pru_read.o pru_read.c
	$(CC) -o pru_read pru_read.o -lpthread -lprussdrv

clean:
	rm -rf pru_start.o pru_stop.o pru_read.o hcsr04.bin

clean_all:
	rm -rf pru_start pru_start.o hcsr04.bin pru_stop pru_stop.o pru_read pru_read.o
