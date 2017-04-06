PRU_ASM = pasm
LDLIBS+= -lpthread -lprussdrv
DTC = dtc
SLOTS = /sys/devices/platform/bone_capemgr/slots

all: hcsr04-00A0.dtbo hcsr04.bin install config

hcsr04-00A0.dtbo: hcsr04.dts
	@echo "\n>> Compiling Driver"
	$(DTC) -O dtb -o hcsr04-00A0.dtbo -b 0 -@ hcsr04.dts

hcsr04.bin: hcsr04.p
	@echo "\n>> Generating PRU binary"
	$(PRU_ASM) -b hcsr04.p
clean:
	rm -rf hcsr04.bin hcsr04-00A0.dtbo 

install: hcsr04-00A0.dtbo hcsr04.bin
	cp hcsr04-00A0.dtbo /lib/firmware/
	echo "hcsr04" > $(SLOTS)
	echo "BB-PWM0" > $(SLOTS)
	echo "BB-PWM1" > $(SLOTS)
	echo "BB-PWM2" > $(SLOTS)
	echo "cape-universala" > $(SLOTS)
	cat $(SLOTS)
config:
	config-pin P9.14 pwm
	config-pin P9.16 pwm
	config-pin P8.13 pwm
	config-pin P8.19 pwm


