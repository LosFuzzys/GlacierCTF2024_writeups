.PHONY: build

# This won't get run automatically. You can use this as a personal helper
# Do not put this in /dist

build:
	# Maybe you don't want to build buildroot each time
	make -C buildroot/
	make -C linux/

build-module:
	make -C linux/
	make -C module/
