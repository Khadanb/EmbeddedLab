ifneq (${KERNELRELEASE},)

# KERNELRELEASE defined: we are being compiled as part of the Kernel
        obj-m := vga_ball.o

	

else

# We are being compiled as a module: use the Kernel build system

	KERNEL_SOURCE := /usr/src/linux-headers-$(shell uname -r)
        PWD := $(shell pwd)

default: module SuperMario_64

SuperMario_64: SuperMario_64.o usbkeyboard.o game_struct.o game_animation.o
	cc $(CFLAGS) -o SuperMario_64 SuperMario_64.o usbkeyboard.o game_struct.o game_animation.o -lusb-1.0 -pthread -lm

SuperMario_64.o: SuperMario_64.c game_struct.h usbkeyboard.h game_animation.h
usbkeyboard.o: usbkeyboard.c usbkeyboard.h
game_struct.o: game_struct.c game_struct.h
game_animation.o: game_animation.c game_animation.h


module:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules

clean:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
	${RM} SuperMario_64

TARFILES = Makefile README vga_ball.h vga_ball.c hello.c
TARFILE = lab3-sw.tar.gz
.PHONY : tar
tar : $(TARFILE)

$(TARFILE) : $(TARFILES)
	tar zcfC $(TARFILE) .. $(TARFILES:%=lab3-sw/%)

endif 
