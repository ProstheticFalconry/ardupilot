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
#define LARGE_DELAY 6000000
#define SMALL_DELAY 1000

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

        // Enable Echo as input (GPIO0)
        MOV r1, GPIO0 | GPIO_OE
        LBBO r0, r1, 0, 4
	SET r0, BIT_ECHO0a
        SET r0, BIT_ECHO0b
        SET r0, BIT_ECHO0c
	SBBO r0, r1, 0, 4

        // Enable Echo as input (GPIO1)
        MOV r1, GPIO1 | GPIO_OE
        LBBO r0, r1, 0, 4
        SET r0, BIT_ECHO1a
        SET r0, BIT_ECHO1b
        SBBO r0, r1, 0, 4

TRIGGER:
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
        
        // initialize flags (all pending, and all waiting for high)
        MOV ECHOS_PENDING, 0x1F
        MOV WAITING_FOR_HIGH, 0x1F
        // Start main loop
        QBA MAIN_LOOP

// Functions that check if Echo pins have gone high
CHECK_IF_HIGH0:
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC SONAR_1, r2, BIT_ECHO0a
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        CLR WAITING_FOR_HIGH, 0
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init0a, r1, 0, 4
        QBA SONAR_1

CHECK_IF_HIGH1:
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC SONAR_2, r2, BIT_ECHO0b
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        CLR WAITING_FOR_HIGH, 1
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init0b, r1, 0, 4
        QBA SONAR_2

CHECK_IF_HIGH2:
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC SONAR_3, r2, BIT_ECHO0c
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        CLR WAITING_FOR_HIGH, 2
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init0c, r1, 0, 4
        QBA SONAR_3

CHECK_IF_HIGH3:
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC SONAR_4, r2, BIT_ECHO1a
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        CLR WAITING_FOR_HIGH, 3
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init1a, r1, 0, 4
        QBA SONAR_4

CHECK_IF_HIGH4:
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBC MAIN_LOOP, r2, BIT_ECHO1b
        
        // ECHO has gone HIGH, so toggle flag and record timer value
        CLR WAITING_FOR_HIGH, 4
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_init1b, r1, 0, 4
        QBA MAIN_LOOP

MAIN_LOOP:
        // Small delay
        MOV r0, SMALL_DELAY
DELAY_S:
        SUB r0, r0, 1
        QBNE DELAY_S, r0, 0

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

        QBEQ EXIT_LOOP, ECHOS_PENDING, 0 
SONAR_alt:
        QBBC SONAR_1, ECHOS_PENDING, 0
        QBBS CHECK_IF_HIGH0, WAITING_FOR_HIGH, 0
        // Must be waiting for ECHO to go low
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS SONAR_1, r2, BIT_ECHO0a
        // ECHO has gone lo, toggle flag, grab time and write to memory
        CLR ECHOS_PENDING, 0
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_final, r1, 0, 4
        SBCO time_init0a, c24, 0, 4
        SBCO time_final, c24, 4, 4
SONAR_1:
        QBBC SONAR_2, ECHOS_PENDING, 1
        QBBS CHECK_IF_HIGH1, WAITING_FOR_HIGH, 1
        // Must be waiting for ECHO to go low
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS SONAR_2, r2, BIT_ECHO0b
        // ECHO has gone lo, toggle flag, grab time and write to memory
        CLR ECHOS_PENDING, 1
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_final, r1, 0, 4
        SBCO time_init0b, c24, 8, 4
        SBCO time_final, c24, 12, 4

SONAR_2:
        QBBC SONAR_3, ECHOS_PENDING, 2
        QBBS CHECK_IF_HIGH2, WAITING_FOR_HIGH, 2
        // Must be waiting for ECHO to go low
        MOV r1, GPIO0 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS SONAR_3, r2, BIT_ECHO0c
        // ECHO has gone lo, toggle flag, grab time and write to memory
        CLR ECHOS_PENDING, 2
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_final, r1, 0, 4
        SBCO time_init0c, c24, 16, 4
        SBCO time_final, c24, 20, 4

SONAR_3:
        QBBC SONAR_4, ECHOS_PENDING, 3
        QBBS CHECK_IF_HIGH3, WAITING_FOR_HIGH, 3
        // Must be waiting for ECHO to go low
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS SONAR_4, r2, BIT_ECHO1a
        // ECHO has gone lo, toggle flag, grab time and write to memory
        CLR ECHOS_PENDING, 3
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_final, r1, 0, 4
        SBCO time_init1a, c24, 24, 4
        SBCO time_final, c24, 28, 4

SONAR_4:
        QBBC MAIN_LOOP, ECHOS_PENDING, 4
        QBBS CHECK_IF_HIGH4, WAITING_FOR_HIGH, 4
        // Must be waiting for ECHO to go low
        MOV r1, GPIO1 | GPIO_DATAIN
        LBBO r2, r1, 0, 4
        QBBS MAIN_LOOP, r2, BIT_ECHO1b
        // ECHO has gone lo, toggle flag, grab time and write to memory
        CLR ECHOS_PENDING, 4
        MOV r1, PRU0_CTRL | CYCLE
        LBBO time_final, r1, 0, 4
        SBCO time_init1b, c24, 32, 4
        SBCO time_final, c24, 36, 4

EXIT_LOOP:


        // Optional Interrupt parent C program
        // MOV r31.b0, PRU0_ARM_INTERRUPT+16        
      
        
        // Delay to allow sonar to stop resonating and sound burst to decay in environment
	MOV r0, LARGE_DELAY
DELAY_L:
	SUB r0, r0, 1
	QBNE DELAY_L, r0, 0

	// Jump back to triggering the sonar
	JMP TRIGGER

	HALT

