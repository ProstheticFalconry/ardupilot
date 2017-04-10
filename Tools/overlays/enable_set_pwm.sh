#!/bin/sh
PWM1=/sys/class/pwm/pwmchip2
PWM2=/sys/class/pwm/pwmchip4

echo "0" > /sys/class/pwm/pwmchip2/export
echo "1" > /sys/class/pwm/pwmchip2/export
echo "0" > /sys/class/pwm/pwmchip4/export
echo "1" > /sys/class/pwm/pwmchip4/export

echo "2000000" > /sys/class/pwm/pwmchip2/pwm0/period
echo "2000000" > /sys/class/pwm/pwmchip2/pwm1/period
echo "2000000" > /sys/class/pwm/pwmchip4/pwm0/period
echo "2000000" > /sys/class/pwm/pwmchip4/pwm1/period

echo "0" > /sys/class/pwm/pwmchip2/pwm0/enable
echo "1" > /sys/class/pwm/pwmchip2/pwm1/enable
echo "0" > /sys/class/pwm/pwmchip4/pwm0/enable
echo "1" > /sys/class/pwm/pwmchip4/pwm1/enable
