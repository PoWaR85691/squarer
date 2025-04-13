DRIVER = squarer

obj-m += $(DRIVER).o 

PWD := $(CURDIR)

.PHONY: all clean

all: $(DRIVER).c
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean