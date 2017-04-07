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
    data_values[0] = 1360;
    data_values[1] = 1360;
    data_values[2] = 860;
    data_values[3] = 1360;
    flight_mode = AUTO;
    output_data.pending = false;
    output_data.type = altitude;
    output_data.values = 0;
}

void RCInput_Falcon::_record_data()
{
    output_data.pending = false;
    char first_char = 1;
    /* Set first character depending on data type */
    switch (output_data.type) {
	case altitude:
	    first_char = ALT_ID;
	    break;
	case compass:
	    first_char = MAG_ID;
	    break;
	case acceleration:
	    first_char = ACCEL_ID;
	    break;
	case gyroscope:
	    break;
	    first_char = GYRO_ID;
	default:
	    printf("ERROR: unrecognized data type in _record_data\n");
	    return;
    }
    char * tmp_buf = (char *) malloc(MAX_FILE_SIZE);
    int len = sprintf(tmp_buf, "%c%5f%c", first_char, output_data.values, char(0));
    printf("before write: tmp_buf = %s (length = %d)\n", tmp_buf, len);
    err = write(_fd, tmp_buf, MAX_FILE_SIZE);
    free(tmp_buf);
    if (err <= 0) {
        printf("ERROR in _record_altitude: err = %d\n\n", err);
    }
}

void RCInput_Falcon::_timer_tick()
{
    /*******************
     GET DATA FROM KERNEL
    ********************/

    /* Read from fly0 file in kernel space (user data) */
    buffer = (char *) malloc(MAX_FILE_SIZE+1);
    err = ::read(_fd, buffer, sizeof(buffer));
    // print buffer if not empty
    if (err == -1) {
	free(buffer);
        AP_HAL::panic("RCInput_Falcon: Error reading in _timer_tick()");
    }
    
    /* Identify what type of information was read */
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
	    printf("YAW\n");
	    row = YAW_OFFSET;
	    break;
	case MODE_MN:
	    printf("MODE\n");
	    change_mode = 1;
	    break;
	default:
	    free(buffer);
	    if (output_data.pending) _record_data();
	    return;
	}
    int length = 1;
    while (buffer[length] != 0){
	length++; // Count how long the row is
    }
    RC_value = a2i(1, length); // Convert string to integer
    if (change_mode == 1) {
	// load a new flight mode 
	flight_mode = RC_value;
	_update_flight_mode(flight_mode);
    } else {
	// Update control_in values from user input
	data_values[row] = RC_value;
	_update_periods(data_values, (uint8_t) CHANNELS);
    }

    /* Debug print statements DELETE */
    /* 
    if (err > 0) {
	for (int i = 0; i < CHANNELS; i++) {
	    printf("datavalues[%u] = %u\n", i, data_values[i]);
	}
	printf("flight mode = %u\n\n", flight_mode);
    }
    */
    free(buffer);

    /*******************
     SEND DATA TO KERNEL
    *******************/
    if (output_data.pending) _record_data();
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


