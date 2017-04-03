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
    _fd = open(path, O_TEXT);
    if (_fd < 0) {
        AP_HAL::panic("RCInput_Falcon: Error opening '%s': %s",
                             path, strerror(errno));
    }
}

RCInput_Falcon::~RCInput_UART()
{
    close(_fd);
}

void RCInput_Falcon::init()
{
	
}

void RCInput_Falcon::_record_altitude(float alt)
{
    // copy string of altitude into tmp_buf
    char * tmp_buf;
    tmp_buf = (char *) malloc(MAX_ROW_LENGTH);
    sprintf(tmp_buf, "%10.2f", alt);

    // shift string 1 to right and replace first byte with ID
    for (int i = 0; i < MAX_ROW_LENGTH - 1; i++) {
	tmp_buf[MAX_ROW_LENGTH-1-i] = tmp_buf[MAX_ROW_LENGTH-2-i]
    }
    tmp_buf[0] = (char) ALTITUDE_ID;
    int err = write(_fd, (void *)tmp_buf, MAX_ROW_LENGTH);
    free(tmp_buf);
    if (err == -1) {
	AP_HAL::panic("RCInput_Falcon: Error writing in _record_altitude");
    }
}

void RCInput_Falcon::_timer_tick()
{
    buffer = malloc(MAX_FILE_SIZE+1);
    err = read(_fd, buffer, sizeof(buffer));

    if (err == -1) {
	free(buffer);
        AP_HAL::panic("RCInput_Falcon: Error reading in _timer_tick()");
    }
    int index = 0;
    int start;
    for (int row = 0 ; row < MAX_ROWS ; row++) {
	firstLetter = buffer[index];
	switch (firstLetter) {
	    case FLY0_MN:
		break;
	    case THROT_MN:
		break;
	    case ROLL_MN:
		break;
	    case PITCH_MN:
		break;
	    case YAW_MN:
		break;
	    case MODE_MN:
		break;
	    default:
		AP_HAL::panic("RCInput_Falcon: Error reading firstLetter");
		break;
	}
	index++;
        start = index;

	while (buffer[index] != 0){
	    index++; // Count how long the row is
	}
	data_values[row] = a2i(start, index - start);
	index++; // step to start of next row
    }

    _update_periods(data_values, (uint8_t) CHANNELS);
    free(buffer);

}

int RCInput_Falcon::a2i(int start, int length)
{
    char * tmp_buff = (char *) malloc(length);
    int tmp_num;
    for (int i = 0; i < length; i++) {
	tmp_buff[i] = (char) buffer[start+i];	    
    }
    tmp_num = atoi(tmp_buff);
    free(tmp_buff);
    return tmp_num;

}


