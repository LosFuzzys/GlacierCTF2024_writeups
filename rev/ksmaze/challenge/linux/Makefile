.PHONY: build

# This won't get run automatically. You can use this as a personal helper
# Do not put this in /dist

KERNEL=linux-5.10.230
URL=https://cdn.kernel.org/pub/linux/kernel/v6.x/${KERNEL}.tar.xz

build:
# Download kernel if not available
ifeq (,$(wildcard ${KERNEL}))
	wget ${URL}
	tar xvf ${KERNEL}.tar.xz
	rm ${KERNEL}.tar.xz
endif
	cp config ${KERNEL}/.config
	make -j $(shell nproc) -C ${KERNEL}
	cp ${KERNEL}/arch/x86/boot/bzImage .
