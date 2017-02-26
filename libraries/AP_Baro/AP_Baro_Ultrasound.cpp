#include "AP_Baro_Ultrasound.h"

#include <AP_HAL/AP_HAL.h>

extern const AP_HAL::HAL& hal;

AP_Baro_Ultrasound::AP_Baro_Ultrasound(AP_Baro &baro) :
	AP_Baro_Backend(baro)
{
	_instance=_frontend.register_sensor();
}

void AP_Baro_Ultrasound::update(void)
{
	_copy_to_frontend(_instance,0,0);
}
