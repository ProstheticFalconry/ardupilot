#pragma once

#include <AP_Common/AP_Common.h>

#include "AP_HAL_Linux.h"
#include "RCInput.h"

#define CHANNELS 5
#define MAX_ROWS 6
#define MAX_ROW_LENGTH 11
#define MAX_FILE_SIZE 66

// RCInput Modes
#define FLY0_MN 0
#define THROT_MN 1
#define ROLL_MN 2
#define PITCH_MN 3
#define YAW_MN 4
#define MODE_MN 5 

// RCOutput Modes
#define COMPASS_ID 0
#define ALTITUDE_ID 1
#define CLIMB_RATE_ID 2


namespace Linux {

class RCInput_UART : public RCInput
{
public:
    RCInput_UART(const char *path);
    ~RCInput_UART();

    void init() override;
    void _timer_tick(void) override;
    void _record_compass(float);
    void _record_altitude(float);
    void _record_climbrate(float);
private:
    int _fd;
    int err;
    void * buffer;
    int mode;
    char firstLetter;
    uint16_t data_values[CHANNELS];

    int a2i(int start, int length);
};

}
