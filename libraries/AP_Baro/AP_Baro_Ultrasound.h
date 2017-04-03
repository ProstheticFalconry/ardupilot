/*fake backend for the ultrasound altimiter. Currently does not work*/
#pragma once

#include "AP_Baro_Backend.h"
#include <pruss/prussdrv.h>
#include <pruss/pruss_intc_mapping.h>


class AP_Baro_Ultrasound : public AP_Baro_Backend
{
public:
	AP_Baro_Ultrasound(AP_Baro &baro);
	void update(void);
	float get_altitude(void);

private:
	uint16_t fakeAlt;
	uint8_t _instance;
	void * pruAddress;
        unsigned int * pruData;
	unsigned int get_timer_diff();
};
