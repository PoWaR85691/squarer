DRIVER = squarer

obj-m += $(DRIVER).o 

PWD := $(CURDIR)

.PHONY: all clean install uninstall

all: $(DRIVER).c
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: $(DRIVER).ko
	insmod $(DRIVER).ko
	tools/mkdevfile.sh /dev/$(DRIVER) $(DRIVER)

uninstall:
	rm -f /dev/$(DRIVER)
	rmmod $(DRIVER)