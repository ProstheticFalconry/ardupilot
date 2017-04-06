#pragma once

#include <AP_Common/AP_Common.h>

#include "AP_HAL_Linux.h"
#include "RCInput.h"

#define CHANNELS 4
#define MAX_ROWS 6
#define MAX_FILE_SIZE 11

// RCInput Channel Offets
#define ROLL_OFFSET 0
#define PITCH_OFFSET 1
#define THROT_OFFSET 2
#define YAW_OFFSET 3

// RCInput Modes
#define THROT_MN 39
#define ROLL_MN 40
#define PITCH_MN 41
#define YAW_MN 42
#define MODE_MN 43

// RCOutput Modes
#define COMPASS_ID 0
#define ALTITUDE_ID 1
#define CLIMB_RATE_ID 2


namespace Linux {

class RCInput_Falcon : public RCInput
{
public:
    RCInput_Falcon(const char *path);
    ~RCInput_Falcon();

    void init() override;
    void _timer_tick(void) override;
    void _record_compass(float);
    void _record_altitude(float);
    void _record_climbrate(float);
private:
    int _fd;
    int err;
    int length;
    char * buffer;
    int change_mode;
    char firstLetter;
    uint16_t RC_value;
    uint16_t data_values[CHANNELS];

    uint16_t a2i(int start, int length);
};

}
