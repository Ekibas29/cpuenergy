obj-m += msr_driver.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	g++ -Wall -g3 -O0 -o energy_info energy_info.cpp cpuenergy.cpp --std=c++11
