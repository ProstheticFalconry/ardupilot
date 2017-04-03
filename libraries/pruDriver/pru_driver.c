#include <stdio.h>
#include <unistd.h>

#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>


int pru_init(void) {

        /* Initialize the PRU */
        printf(">> Initializing PRU\n");
        tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
        prussdrv_init();

        /* Open PRU Interrupt */
        if (prussdrv_open (PRU_EVTOUT_0)) {
                // Handle failure
                fprintf(stderr, ">> PRU init open failed\n");
                return 1;
        }
        /* Get the interrupt initialized */
        prussdrv_pruintc_init(&pruss_intc_initdata);

        /* Execute code on PRU */
        printf(">> Executing HCSR-04 code\n");
        prussdrv_exec_program(0, "hcsr04.bin");
        return 0;
}
