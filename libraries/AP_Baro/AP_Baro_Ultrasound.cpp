#include "AP_Baro_Ultrasound.h"
#include <AP_HAL/AP_HAL.h>
#include <math.h>
#include <stdio.h>
#include <pruDriver/pru_driver.h>

#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>

#define CYCLES_PER_SEC 200000000 // 200 MHz
#define SPEED_OF_SOUND 343 // in m/s

extern const AP_HAL::HAL& hal;

AP_Baro_Ultrasound::AP_Baro_Ultrasound(AP_Baro &baro) :
	AP_Baro_Backend(baro)
{
	_instance=_frontend.register_sensor();
	if (prussdrv_open (PRU_EVTOUT_0)) {
        	// Handle failure
                printf(">> PRU open failed\n");
	}

	prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruAddress);
        pruData = (unsigned int *) pruAddress;
	printf("PRU address successfully assigned\n\n");
}


void AP_Baro_Ultrasound::update()
{
	_copy_to_frontend(_instance,get_altitude(),25);
}

unsigned int AP_Baro_Ultrasound::get_timer_diff()
{
	unsigned int t2 = pruData[1];
	unsigned int t1 = pruData[0];
	if (t2 >= t1) {
		return t2 - t1;
	}
	else {
		return 1;
	}
}

float AP_Baro_Ultrasound::get_altitude(void)
{
	float dt = (float) get_timer_diff();
	float dist = dt * SPEED_OF_SOUND / (2 * CYCLES_PER_SEC);
	return dist;
}
