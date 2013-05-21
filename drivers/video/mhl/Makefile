#
# Makefile for MHL sii8334 chip drivers.

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

#ccflags-y := -I$(src)/include
#ccflags-y += -I$(src)/platform/hal
#ccflags-y += -I$(src)/component/mhl_tx
#ccflags-y += -I$(src)/driver/cra_drv
#ccflags-y += -I$(src)/driver/sii8334_drv
#ccflags-y += -I$(src)/linux_kernel/kernel_module
ccflags-y += -DMAKE_8334_DRIVER
ccflags-$(CONFIG_DEBUG_DRIVER) += -DDEBUG


obj-$(CONFIG_MHL_Sii8334) += sii8334drv.o

#----------------- Component layer --------------------------------
sii8334drv-objs  += si_cra.o
sii8334drv-objs  += si_mhl_tx_rcp_table.o
#----------------- Platform layer ---------------------------------
sii8334drv-objs  += si_drv_mhl_tx.o
sii8334drv-objs  += si_mhl_tx.o
sii8334drv-objs  += MHL_SiI8334.o


else

# Normal Makefile

KERNELDIR=/home/netclic/src/linux-2.6.36
ARCH=arm

PWD := $(shell pwd)


.PHONY: sii8334_clean


sii8334_release:
	make -C $(KERNELDIR) M=$(PWD) CONFIG_SII8334_MHL_TX=m modules
	$(CROSS_COMPILE)strip --strip-debug sii8334drv.ko

sii8334_debug:
	make -C $(KERNELDIR) M=$(PWD) CONFIG_SII8334_MHL_TX=m CONFIG_DEBUG_DRIVER=y modules

sii8334_clean:
	make -C $(KERNELDIR) M=$(PWD) CONFIG_SII8334_MHL_TX=m clean
endif
