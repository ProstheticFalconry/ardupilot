#include <stdio.h>
#include <unistd.h>

#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>

#define CYCLES_PER_SEC 200000000
#define SPEED_OF_SOUND 34000 // in cm/s

int main(void) {
	/* Open PRU */
        if (prussdrv_open (PRU_EVTOUT_0)) {
                // Handle failure
                fprintf(stderr, ">> PRU open failed\n");
                return 1;
        }
        /* Get the interrupt initialized */
        //prussdrv_pruintc_init(&pruss_intc_initdata);

        /* Get pointers to PRU local memory */
        void *pruDataMem;
        prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruDataMem);
        unsigned int *pruData = (unsigned int *) pruDataMem;
	
	double range_0 = 0;
        pruData[0] = 0;
        pruData[1] = 0;

	for (int i = 0; i < 40 ;i++) {
		range_0 = (double) (pruData[1] - pruData[0]) * SPEED_OF_SOUND / (2 * CYCLES_PER_SEC);

	        printf("(%2d) Sonar 1: Distance = %.2lf cm\n", i, range_0);
        	sleep(1);
	}
	return 0;
}
