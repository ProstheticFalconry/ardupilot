#include "Copter.h"
#include <stdio.h>
/*
  mavlink motor test - implements the MAV_CMD_DO_MOTOR_TEST mavlink command so that the GCS/pilot can test an individual motor or flaps
                       to ensure proper wiring, rotation.
 */

// motor test definitions
#define MOTOR_TEST_PWM_MIN              800     // min pwm value accepted by the test
#define MOTOR_TEST_PWM_MAX              2200    // max pwm value accepted by the test
#define MOTOR_TEST_TIMEOUT_MS_MAX       30000   // max timeout is 30 seconds

static uint32_t motor_test_start_internal_ms = 0;        // system time the motor test began
static uint32_t motor_test_timeout_internal_ms = 0;      // test will timeout this many milliseconds after the motor_test_start_ms
static uint8_t motor_test_internal_seq = 0;              // motor sequence number of motor being tested
static uint16_t motor_test_internal_throttle_value = 0;  // throttle to be sent to motor, value depends upon it's type
// motor_test_output - checks for timeout and sends updates to motors objects
void Copter::motor_test_output_internal()
{
    printf("Setting motors, motor test is %d\n",ap.motor_test_internal);
    if(!ap.motor_test_internal)
	return;
	
    // check for test timeout
//    if ((AP_HAL::millis() - motor_test_start_internal_ms) >= motor_test_timeout_internal_ms) {
        // stop motor test
      //  motor_test_internal_stop();
    //} else {
        // sanity check throttle values
        if (motor_test_internal_throttle_value >= MOTOR_TEST_PWM_MIN && motor_test_internal_throttle_value <= MOTOR_TEST_PWM_MAX ) {
            printf("\nsetting motors to %d\n",motor_test_internal_throttle_value);
            motors->output_test(motor_test_internal_seq, motor_test_internal_throttle_value);
        } else {
//            motor_test_internal_stop();
        }
    //}
}

uint8_t Copter::motor_test_internal_start(uint8_t mot_seq, uint32_t mot_throt)
{
    // if test has not started try to start it
    printf("initializing motor test. test value is %d\n",ap.motor_test_internal);
    if (!ap.motor_test_internal) {
        
            // start test
            ap.motor_test_internal = true;
	    printf("in motor test init. test value is %d\n",ap.motor_test_internal);
            // enable and arm motors
            if (!motors->armed()) {
                init_rc_out();
                enable_motor_output();
                motors->armed(true);
            }

            // disable throttle, battery and gps failsafe
            g.failsafe_throttle = FS_THR_DISABLED;
            g.failsafe_battery_enabled = FS_BATT_DISABLED;
            g.failsafe_gcs = FS_GCS_DISABLED;

    // set timeout
    motor_test_start_internal_ms = AP_HAL::millis();
    motor_test_timeout_internal_ms = MOTOR_TEST_TIMEOUT_MS_MAX;

    // store required output
    motor_test_internal_seq = mot_seq;
    motor_test_internal_throttle_value = mot_throt;
    }
    return 0;
}
// motor_test_stop - stops the motor test
void Copter::motor_test_internal_stop()
{
    // exit immediately if the test is not running
    if (!ap.motor_test_internal) {
        return;
    }

    // flag test is complete
    ap.motor_test_internal = false;

    // disarm motors
    motors->armed(false);

    // reset timeout
    motor_test_start_internal_ms = 0;
    motor_test_timeout_internal_ms = 0;

    // re-enable failsafes
    g.failsafe_throttle.load();
    g.failsafe_battery_enabled.load();
    g.failsafe_gcs.load();

    // turn off notify leds
    AP_Notify::flags.esc_calibration = false;
}
