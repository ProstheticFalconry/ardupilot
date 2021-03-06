#include "Copter.h"

/*
  compass/motor interference calibration
 */

// setup_compassmot - sets compass's motor interference parameters
void Copter::mavlink_compassmot(void)
{
#if FRAME_CONFIG == HELI_FRAME
    // compassmot not implemented for tradheli
    printf("Compassmot.cpp: Wrong framte type\n");
    return;
#else
    int8_t   comp_type;                 // throttle or current based compensation
    Vector3f compass_base[COMPASS_MAX_INSTANCES];           // compass vector when throttle is zero
    Vector3f motor_impact[COMPASS_MAX_INSTANCES];           // impact of motors on compass vector
    Vector3f motor_impact_scaled[COMPASS_MAX_INSTANCES];    // impact of motors on compass vector scaled with throttle
    Vector3f motor_compensation[COMPASS_MAX_INSTANCES];     // final compensation to be stored to eeprom
    float    throttle_pct;              // throttle as a percentage 0.0 ~ 1.0
    float    throttle_pct_max = 0.0f;   // maximum throttle reached (as a percentage 0~1.0)
    float    current_amps_max = 0.0f;   // maximum current reached
    float    interference_pct[COMPASS_MAX_INSTANCES];       // interference as a percentage of total mag field (for reporting purposes only)
    uint32_t last_run_time;
    uint32_t last_send_time;
    bool     updated = false;           // have we updated the compensation vector at least once
    uint8_t  command_ack_start = command_ack_counter;

    // exit immediately if we are already in compassmot
    if (ap.compass_mot) {
        printf("Compassmot.cpp: already in compassmot\n");
        return;
    }else{
        ap.compass_mot = true;
    }

    // initialise output
    for (uint8_t i=0; i<COMPASS_MAX_INSTANCES; i++) {
        interference_pct[i] = 0.0f;
    }

    // check compass is enabled
    if (!g.compass_enabled) {
        printf("Compassmot.cpp: compass disabled\n");
        ap.compass_mot = false;
        return;
    }

    // check compass health
    compass.read();
    printf("Compass getcount %d\n",compass.get_count());
    for (uint8_t i=0; i<compass.get_count(); i++) {
        if (!compass.healthy(i)) {
            printf("Compassmot.cpp: compass unhealthy\n");
            ap.compass_mot = false;
            return;
        }
    }

    // check if radio is calibrated
    /*arming.pre_arm_rc_checks(true);
    if (!ap.pre_arm_rc_check) {
        
        ap.compass_mot = false;
        return MAV_RESULT_TEMPORARILY_REJECTED;
    }*/

    // check throttle is at zero
    read_radio();
    if (channel_throttle->get_control_in() != 0) {
        printf("Compassmot.cpp: Throttle not zero\n");
        ap.compass_mot = false;
        return;
    }

    // check we are landed
    /*if (!ap.land_complete) {
        gcs_chan[chan-MAVLINK_COMM_0].send_text(MAV_SEVERITY_CRITICAL, "Not landed");
        ap.compass_mot = false;
        return MAV_RESULT_TEMPORARILY_REJECTED;
    }*/

    // disable cpu failsafe
    failsafe_disable();

    // initialise compass
//    init_compass();

    // default compensation type to use current if possible
    comp_type = AP_COMPASS_MOT_COMP_THROTTLE;

    // send back initial ACK
    //mavlink_msg_command_ack_send(chan, MAV_CMD_PREFLIGHT_CALIBRATION,0);

    // flash leds
    //AP_Notify::flags.esc_calibration = true;

    // warn user we are starting calibration
    printf("Starting calibration\n");

    // disable throttle and battery failsafe
    g.failsafe_throttle = FS_THR_DISABLED;
    g.failsafe_battery_enabled = FS_BATT_DISABLED;

    // disable motor compensation
    compass.motor_compensation_type(AP_COMPASS_MOT_COMP_DISABLED);
    for (uint8_t i=0; i<compass.get_count(); i++) {
        compass.set_motor_compensation(i, Vector3f(0,0,0));
    }

    // get initial compass readings
    last_run_time = millis();
    while ( millis() - last_run_time < 500 ) {
        compass.accumulate();
    }
    compass.read();

    // store initial x,y,z compass values
    // initialise interference percentage
    for (uint8_t i=0; i<compass.get_count(); i++) {
        compass_base[i] = compass.get_field(i);
        interference_pct[i] = 0.0f;
    }

    // enable motors and pass through throttle
    init_rc_out();
    enable_motor_output();
    motors->armed(true);

    // initialise run time
    last_run_time = millis();
    last_send_time = millis();
    compass.read();
    // main run while there is no user input and the compass is healthy
    while (channel_roll->get_control_in()==0 && compass.healthy(0) && motors->armed()) {
        printf("looping \n");
        // 50hz loop
        if (millis() - last_run_time < 20) {
            // grab some compass values
            compass.accumulate();
            hal.scheduler->delay(5);
            continue;
        }
        last_run_time = millis();

        // read radio input
        read_radio();
        
        // pass through throttle to motors
        motors->set_throttle_passthrough_for_esc_calibration(channel_throttle->get_control_in() / 1000.0f);
        
        // read some compass values
        compass.read();
        
        // read current
        read_battery();
        
        // calculate scaling for throttle
        throttle_pct = (float)channel_throttle->get_control_in() / 1000.0f;
        throttle_pct = constrain_float(throttle_pct,0.0f,1.0f);
        // if throttle is near zero, update base x,y,z values
        if (throttle_pct <= 0.0f) {
            for (uint8_t i=0; i<compass.get_count(); i++) {
                compass_base[i] = compass_base[i] * 0.99f + compass.get_field(i) * 0.01f;
            }

            // causing printing to happen as soon as throttle is lifted
        } else {

            // calculate diff from compass base and scale with throttle
            for (uint8_t i=0; i<compass.get_count(); i++) {
                motor_impact[i] = compass.get_field(i) - compass_base[i];
            }

            // throttle based compensation
            if (comp_type == AP_COMPASS_MOT_COMP_THROTTLE) {
                // for each compass
                for (uint8_t i=0; i<compass.get_count(); i++) {
                    // scale by throttle
                    motor_impact_scaled[i] = motor_impact[i] / throttle_pct;
                    // adjust the motor compensation to negate the impact
                    motor_compensation[i] = motor_compensation[i] * 0.99f - motor_impact_scaled[i] * 0.01f;
                }

                updated = true;
            } else {
                // for each compass
                for (uint8_t i=0; i<compass.get_count(); i++) {
                    // current based compensation if more than 3amps being drawn
                    motor_impact_scaled[i] = motor_impact[i] / battery.current_amps();
                
                    // adjust the motor compensation to negate the impact if drawing over 3amps
                    if (battery.current_amps() >= 3.0f) {
                        motor_compensation[i] = motor_compensation[i] * 0.99f - motor_impact_scaled[i] * 0.01f;
                        updated = true;
                    }
                }
            }

            // calculate interference percentage at full throttle as % of total mag field
                for (uint8_t i=0; i<compass.get_count(); i++) {
                    // interference is impact@fullthrottle / mag field * 100
                    interference_pct[i] = motor_compensation[i].length() / (float)arming.compass_magfield_expected() * 100.0f;
                }

            // record maximum throttle and current
            throttle_pct_max = MAX(throttle_pct_max, throttle_pct);
            current_amps_max = MAX(current_amps_max, battery.current_amps());

        }
        if (AP_HAL::millis() - last_send_time > 500) {
            last_send_time = AP_HAL::millis();
            printf("Throttle: %u, Interference: %f, x comp: %f, y comp: %f, z comp: %f",
                                               channel_throttle->get_control_in(),
                                               interference_pct[compass.get_primary()],
                                               motor_compensation[compass.get_primary()].x,
                                               motor_compensation[compass.get_primary()].y,
                                               motor_compensation[compass.get_primary()].z);
    	}
    }
    printf("\nroll channel is %d,the compass health is %d\n",channel_roll->get_control_in(),compass.healthy(0));	
// stop motors
    motors->output_min();
    motors->armed(false);

    // set and save motor compensation
    if (updated) {
        compass.motor_compensation_type(comp_type);
        for (uint8_t i=0; i<compass.get_count(); i++) {
            compass.set_motor_compensation(i, motor_compensation[i]);
        }
        compass.save_motor_compensation();
        // display success message
        printf("Calibration successful\n");
    } else {
        // compensation vector never updated, report failure
        printf("Failed\n");
        compass.motor_compensation_type(AP_COMPASS_MOT_COMP_DISABLED);
    }

    // display new motor offsets and save
    report_compass();

    // turn off notify leds
    AP_Notify::flags.esc_calibration = false;

    // re-enable cpu failsafe
    failsafe_enable();

    // re-enable failsafes
    g.failsafe_throttle.load();
    g.failsafe_battery_enabled.load();

    // flag we have completed
    ap.compass_mot = false;

    return;
#endif  // FRAME_CONFIG != HELI_FRAME
}

