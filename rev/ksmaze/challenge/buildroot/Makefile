.PHONY: build

# This won't get run automatically. You can use this as a personal helper
# Do not put this in /dist

BUILDROOT=buildroot-2024.02.4

build:
# Download buildroot if not available
ifeq (,$(wildcard ${BUILDROOT}))
	wget https://www.buildroot.org/downloads/${BUILDROOT}.tar.gz
	tar xvzf ${BUILDROOT}.tar.gz
	rm ${BUILDROOT}.tar.gz
endif

	cp config ${BUILDROOT}/.config
	make -C ${BUILDROOT} -j$(shell nproc)
	cp ${BUILDROOT}/output/images/rootfs.ext2 .
	qemu-img convert -f raw -O qcow2 rootfs.ext2 rootfs.qcow2
	rm rootfs.ext2

genimg:
	cp ${BUILDROOT}/output/images/rootfs.ext2 .
	qemu-img convert -f raw -O qcow2 rootfs.ext2 rootfs.qcow2
