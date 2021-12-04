M_NAME := filevault
obj-m := ${M_NAME}.o
${M_NAME}-y := main.o tools.o hooks.o
KERNEL := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C ${KERNEL} M=${PWD} modules

clean:
	make -C ${KERNEL} M=${PWD} clean

insert:
	insmod ${M_NAME}.ko

remove:
	rmmod ${M_NAME}

.PHONY: all clean insert remove