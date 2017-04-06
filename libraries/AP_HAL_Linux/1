/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "RCOutput_Sysfs.h"

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>

namespace Linux {

RCOutput_Sysfs::RCOutput_Sysfs(uint8_t chip, uint8_t channel_base, uint8_t channel_count)
    : _chip(chip)
    , _chip_list(0)
    , _chip_count(0)
    , _channel_base(channel_base)
    , _channel_count(channel_count)
    , _pwm_channels(new PWM_Sysfs_Base *[_channel_count])
    , _pending(new uint16_t[_channel_count])
    , _multichip(false)
{
}

RCOutput_Sysfs::RCOutput_Sysfs(uint8_t *chip_list, uint8_t chip_count, uint8_t channel_base, uint8_t channel_count_per_chip)
    : _chip_list(chip_list)
    , _chip(0)
    , _chip_count(chip_count)
    , _channel_base(channel_base)
    , _channel_count(channel_count_per_chip)
    , _pwm_channels(new PWM_Sysfs_Base *[_chip_count*_channel_count])
    , _pending(new uint16_t[_chip_count*_channel_count])
    , _multichip(true)
{
}


RCOutput_Sysfs::~RCOutput_Sysfs()
{
    for (uint8_t i = 0; i < _channel_count; i++) {
        delete _pwm_channels[i];
    }

    delete _pwm_channels;
}

void RCOutput_Sysfs::init()
{
    if(!_multichip){
    	for (uint8_t i = 0; i < _channel_count; i++) {
        	_pwm_channels[i] = new PWM_Sysfs(_chip, _channel_base+i);
        	if (!_pwm_channels[i]) {
            	AP_HAL::panic("RCOutput_Sysfs_PWM: Unable to setup PWM pin.");
        	}
        	init_channel(_pwm_channels[i]);
    	}
    } else {
	uint8_t channel_index;
    	for (uint8_t i = 0; i < _chip_count; i++) {
		for (uint8_t j = 0; j < _channel_count; j++){
			channel_index = (i*_channel_count) + j;
			_pwm_channels[channel_index] = new PWM_Sysfs(*(_chip_list+i), _channel_base+j);
			if (!_pwm_channels[channel_index]) {
			AP_HAL::panic("RCOutput_Sysfs_PWM: UNable to setup PWM pin.");
			}
			init_channel(_pwm_channels[channel_index]);
			

		}
	}

    }
}

void RCOutput_Sysfs::set_freq(uint32_t chmask, uint16_t freq_hz)
{
    for (uint8_t i = 0; i < _channel_count; i++) {
        if (chmask & 1 << i) {
            _pwm_channels[i]->set_freq(freq_hz);
        }
    }
}

uint16_t RCOutput_Sysfs::get_freq(uint8_t ch)
{
    if (ch >= _channel_count) {
        return 0;
    }

    return _pwm_channels[ch]->get_freq();
}

void RCOutput_Sysfs::enable_ch(uint8_t ch)
{
    if (ch >= _channel_count) {
        return;
    }

    _pwm_channels[ch]->enable(true);
}

void RCOutput_Sysfs::disable_ch(uint8_t ch)
{
    if (ch >= _channel_count) {
        return;
    }

    _pwm_channels[ch]->enable(false);
}

void RCOutput_Sysfs::write(uint8_t ch, uint16_t period_us)
{
    if (ch >= _channel_count) {
        return;
    }
    if (_corked) {
        _pending[ch] = period_us;
        _pending_mask |= (1U<<ch);
    } else {
        _pwm_channels[ch]->set_duty_cycle(usec_to_nsec(period_us));
    }
}

uint16_t RCOutput_Sysfs::read(uint8_t ch)
{
    if (ch >= _channel_count) {
        return 1000;
    }

    return nsec_to_usec(_pwm_channels[ch]->get_duty_cycle());
}

void RCOutput_Sysfs::read(uint16_t *period_us, uint8_t len)
{
    for (int i = 0; i < MIN(len, _channel_count); i++) {
        period_us[i] = read(i);
    }
    for (int i = _channel_count; i < len; i++) {
        period_us[i] = 1000;
    }
}

void RCOutput_Sysfs::cork(void)
{
    _corked = true;
}

void RCOutput_Sysfs::push(void)
{
    for (uint8_t i=0; i<_channel_count; i++) {
        if ((1U<<i) & _pending_mask) {
            _pwm_channels[i]->set_duty_cycle(usec_to_nsec(_pending[i]));
        }
    }
    _pending_mask = 0;
    _corked = false;
}

void RCOutput_Sysfs::init_channel(PWM_Sysfs_Base *pwm_chan){
	pwm_chan->init();
        pwm_chan->enable(false);

        /* Set the initial frequency */
        pwm_chan->set_freq(500);
        pwm_chan->set_duty_cycle(0);
        pwm_chan->set_polarity(PWM_Sysfs::Polarity::NORMAL);
}
    
}
    
