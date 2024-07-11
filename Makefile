obj-m += spec_watchpoint_collector.o
PWD := $(CURDIR) 

all: 
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean: 
		make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
