KVERS = $(shell uname -r)

#Kernel modules
obj-m += tasklet_test.o paramirq.o #char.o #hello.o

#specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0

build:kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
