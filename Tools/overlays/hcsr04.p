// Define the entry point of the program
.origin 0
.entrypoint START

// Address of the io controllers for GPIO1, GPIO2 and GPIO3
#define GPIO0 0x44E07000
#define GPIO1 0x4804C000
// #define GPIO02 0x481AC000
// #define GPIO03 0x481AE000

// Address of the PRUn_CTRL registers
#define PRU0_CTRL 0x22000
#define PRU1_CTRL 0x24000

// Offset address for the output enable register of the gpio controller
#define GPIO_OE 0x134

// Offset address for the data in/out register of the gpio controller
#define GPIO_DATAIN 0x138
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

// Offset address for the CYCLE register of PRU controller
#define CYCLE 0xC

// Bit for enabling the cycle counter on PRU_CTRL register
#define CTR_EN 3

// TRIGGER PIN
// gpio1[12] P8_12 gpio44 0x030
#define BIT_TRIGGER44 0x0C

// ECHO PINS
// gpio1[15] P8_15 gpio47 0x
#define BIT_ECHO47 0x0F
//gpio1[14] P8_16 gpio46 0x
#define BIT_ECHO46 0x0E
//gpio0[2] P9_22 gpio2 0x
#define BIT_ECHO2 0x02
// gpio0[14] P9_26 gpio14 0x0
#define BIT_ECHO14 0x0E
// gpio0[15] P9_24 gpio15 0x0
#define BIT_ECHO15 0x0F

// PRU interrupt for PRU0
#define PRU0_ARM_INTERRUPT 19

// TRIGGER and ECHO PINS Being Used 
#define BIT_TRIGGER BIT_TRIGGER44
#define BIT_ECHO1a BIT_ECHO47
#define BIT_ECHO1b BIT_ECHO46
#define BIT_ECHO0a BIT_ECHO2
#define BIT_ECHO0b BIT_ECHO14
#define BIT_ECHO0c BIT_ECHO15

#define MAX_TIME 2230
#define WAIT_TIME 6000000

// Register reserved for holding initial clock values
#define time_init0a r6
#define time_init0b r7
#define time_init0c r8
#define time_init1a r9
#define time_init1b r10
#define time_final r11

// Flags. 1 means does not have a value yet
#define ECHOS_PENDING r12
// Flags. 1 means waiting for pin to go HIGH, 0 means waiting to go LOW
#define WAITING_FOR_HIGH r13

START:

	// Clear the STANDBY_INIT bit in the SYSCFG register
	// otherwise the PRU will not be able to write outside the PRU memory space
	// and to the Beaglebone pins
	LBCO r0, C4, 4, 4
	CLR r0, r0, 4
	SBCO r0, C4, 4, 4
	
	// Make constant 24 (c24) point to the beginning of PRU0 data ram
        // 0x22000 is PRU_CTRL Registers. 0x20 is the offset for register that determines C24 address
        // SBBO copies 4 bytes of r0 to r1
	MOV r0, 0x00000000
	MOV r1, 0x22020
	SBBO r0, r1, 0, 4

        // Enable Cycle Counter. 0x2200 is address of PRU_CTRL register. CTR_EN is bit the enables the counter
        MOV r1, PRU0_CTRL
        LBBO r0, r1, 0, 4
        SET r0, CTR_EN
        SBBO r0, r1, 0, 4

	// Enable trigger as output
	MOV r1, GPIO1 | GPIO_OE
	LBBO r0, r1, 0, 4
	CLR r0, BIT_TRIGGER
        SBBO r0, r1, 0, 4

        // Enable Echo as input (GPIO1)
        MOV r1, GPIO1 | GPIO_OE
        LBBO r0, r1, 0, 4
        SET r0, BIT_ECHO1b
        SBBO r0, r1, 0, 4

TRIGGER:
        // Reset clock cycle counter
        MOV r1, PRU0_CTRL
        LBBO r0, r1, 0, 4
        CLR r0, CTR_EN
        SBBO r0, r1, 0, 4
        
        MOV r3, PRU0_CTRL | CYCLE
        MOV r2, 1
        SBBO r2, r3, 0, 4

        SET r0, CTR_EN
        SBBO r0, r1, 0, 4

        // Fire the sonar
	// Set trigger pin to high
	MOV r2, 1<<BIT_TRIGGER
	MOV r3, GPIO1 | GPIO_SETDATAOUT
	SBBO r2, r3, 0, 4
	
	// Delay 10 microseconds (200 MHz / 2 instructions = 10 ns per loop, 10 us = 1000 loops) 
	MOV r0, 1000
TRIGGER_DELAY:
	SUB r0, r0, 1
	QBNE TRIGGER_DELAY, r0, 0
	
	// Set trigger pin to low
	MOV r2, 1<<BIT_TRIGGER
	MOV r3, GPIO1 | GPIO_CLEARDATAOUT
	SBBO r2, r3, 0, 4

// Wait for echo to go high        

// Functions that check if Echo pins have gone high
WAIT_FOR_HIGH:
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC WAIT_FOR_HIGH, r2, BIT_ECHO1b
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init1b, r1, 0, 4

WAIT_FOR_LOW:
        // Delay by 100 * 2 instructions * 5 ns = 1 microsecond
        MOV r0, 100
DELAY:
        SUB r0, r0, 1
        QBNE DELAY, r0, 0
        // End of Delay

        // Check if Echo has gone low
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS WAIT_FOR_LOW, r2, BIT_ECHO1b
        
        // ECHO has gone low, grab time
        MOV r1, PRU0_CTRL | CYCLE
        
        // Write times to memory
        LBBO time_final, r1, 0, 4
        SBCO time_init1b, c24, 0, 4
        SBCO time_final, c24, 4, 4

        // Optional Interrupt parent C program
        // MOV r31.b0, PRU0_ARM_INTERRUPT+16        
              
        // Delay to allow sonar to stop resonating 
	MOV r0, WAIT_TIME
RESET_DELAY:
	SUB r0, r0, 1
	QBNE RESET_DELAY, r0, 0

	// Jump back to triggering the sonar
	JMP TRIGGER

	HALT

