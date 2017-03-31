#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>
#include <stdio.h>

int main(void) {
        /* Open PRU Interrupt */
        if (prussdrv_open (PRU_EVTOUT_0)) {
                // Handle failure
                fprintf(stderr, ">> PRU open failed\n");
                return 1;
        }

        /* Disable PRU and close memory mapping*/
        prussdrv_pru_disable(0);
        prussdrv_exit();
        printf(">> PRU Disabled.\r\n");

        return (0);

}


