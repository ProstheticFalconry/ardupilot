/*fake backend for the ultrasound altimiter. Currently does not work*/
#pragma once

#include "AP_Baro_Backend.h"

class AP_Baro_Ultrasound : public AP_Baro_Backend
{
public:
	AP_Baro_Ultrasound(AP_Baro &baro);
	void update(void);

private:
	uint16_t fakeAlt;
	uint8_t _instance;
}
