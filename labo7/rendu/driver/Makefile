### Makefile for driver ###
KERNELDIR := /home/nox/Documents/Cours/BA8/SCF/Labo/scf_2026/labo6/linux/linux
TOOLCHAIN := /opt/toolchain/linaro-arm-linux-gnueabihf-11.3.1/bin/arm-linux-gnueabihf-

obj-m := axi4lite_driver.o

PWD := $(shell pwd)
WARN := -W -Wall -Wstrict-prototypes -Wmissing-prototypes

all: axi4lite_driver

axi4lite_driver:
	@echo "Building with kernel sources in $(KERNELDIR)"
	$(MAKE) ARCH=arm CROSS_COMPILE=$(TOOLCHAIN) -C $(KERNELDIR) M=$(PWD) ${WARN}

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers
