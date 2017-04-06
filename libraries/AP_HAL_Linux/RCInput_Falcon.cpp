#include "RCInput_Falcon.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <AP_HAL/AP_HAL.h>


using namespace Linux;

RCInput_Falcon::RCInput_Falcon(const char *path)
{
    _fd = open(path, O_RDWR);
    if (_fd < 0) {
        AP_HAL::panic("RCInput_Falcon: Error opening '%s': %s", path, strerror(errno));
    }
}

RCInput_Falcon::~RCInput_Falcon(void)
{
    close(_fd);
}

void RCInput_Falcon::init()
{
    for (int i = 0 ; i < CHANNELS - 1 ; i++) {
	data_values[i] = 860;
    }
    data_values[CHANNELS - 1] = 0;
}

void RCInput_Falcon::_record_altitude(float alt)
{
    // copy string of altitude into tmp_buf
    char * tmp_buf;
    tmp_buf = (char *) malloc(MAX_FILE_SIZE);
    sprintf(tmp_buf, "%10.2f", alt);

    // shift string 1 to right and replace first byte with ID
    for (int i = 0; i < MAX_FILE_SIZE - 1; i++) {
	tmp_buf[MAX_FILE_SIZE-1-i] = tmp_buf[MAX_FILE_SIZE-2-i];
    }
    tmp_buf[0] = (char) ALTITUDE_ID;
    int err = write(_fd, (void *)tmp_buf, MAX_FILE_SIZE);
    free(tmp_buf);
    if (err == -1) {
	AP_HAL::panic("RCInput_Falcon: Error writing in _record_altitude");
    }
}

void RCInput_Falcon::_timer_tick()
{
    buffer = (char *) malloc(MAX_FILE_SIZE+1);
    err = ::read(_fd, buffer, sizeof(buffer));

    if (err == -1) {
	free(buffer);
        AP_HAL::panic("RCInput_Falcon: Error reading in _timer_tick()");
    }
    
    firstLetter = buffer[0];
    int row = -1;
    change_mode = 0;
    switch (firstLetter) {
	case ROLL_MN:
	    row = ROLL_OFFSET;
	    break;
	case PITCH_MN:
	    row = PITCH_OFFSET;
	    break;
	case THROT_MN:
	    row = THROT_OFFSET;
	    break;
	case YAW_MN:
	    row = YAW_OFFSET;
	    break;
	case MODE_MN:
	    change_mode = 1;
	    break;
	default:
	    free(buffer);
	    return;
	}
	length = 1;
	while (buffer[length] != 0){
	    length++; // Count how long the row is
	}
	RC_value = a2i(1, length);
    if (change_mode == 1) {
	_update_flight_mode(RC_value);
    } else {
	data_values[row] = RC_value;
	_update_periods(data_values, (uint8_t) CHANNELS);
    
    }
    free(buffer);

}

uint16_t RCInput_Falcon::a2i(int start, int len)
{
    char * tmp_buff = (char *) malloc(len);
    int tmp_num;
    for (int i = 0; i < len; i++) {
	tmp_buff[i] = (char) buffer[start+i];	    
    }
    tmp_num = atoi(tmp_buff);
    free(tmp_buff);
    return (uint16_t) tmp_num;

}


