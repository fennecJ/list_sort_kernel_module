obj-m += list_sort.o
KBUILD_EXTRA_SYMBOLS += $(PWD)/list_impls/Module.symvers
# export KBUILD_EXTRA_SYMBOLS

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean