
obj-m :=led_driver.o  

KDIR = /nfs/cb2/rfs-4.04/lib/modules/4.0.4/build
#KDIR =/nfs/cubie/bt-rfs/linux-sunxi-sunxi-3.4.61


all:test_led
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
test_led:test_led.o
	arm-linux-gcc -o test_led test_led.c

clean:                    
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean
	@rm -rf test_led *.o
    
