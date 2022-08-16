CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/extra
TARGET = infotecs

obj-m := $(TARGET).o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

load:
	insmod $(DEST)/$(TARGET).ko

unload: 
	rmmod $(TARGET)

help:
	@echo 'Targets: '
	@echo 'all: building module for current kernel'
	@echo 'install: installing and sigining module'
	@echo 'load: insmod'
	@echo 'unload: rmmod'
	@echo 'clean: cleaning this dir'

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order *.ko *.mod
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

