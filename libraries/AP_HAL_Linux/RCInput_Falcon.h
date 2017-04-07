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


// RCInput data types
#define THROT_MN 39
#define ROLL_MN 40
#define PITCH_MN 41
#define YAW_MN 42
#define MODE_MN 43

// RCOutput Modes
#define MAG_ID 44
#define ACCEL_ID 45
#define ALT_ID 46
#define GYRO_ID 47


namespace Linux {

class RCInput_Falcon : public RCInput
{
public:
    RCInput_Falcon(const char *path);
    ~RCInput_Falcon();
    void init() override;
    void _timer_tick(void) override;
private:
    void _record_data(void);
    
    int _fd;
    int err;
    char * buffer;
    int change_mode;
    char firstLetter;
    uint16_t RC_value;
    uint16_t flight_mode;
    uint16_t data_values[CHANNELS];

    uint16_t a2i(int start, int length);
};

}
