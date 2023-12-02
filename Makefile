.PHONY: all debug build

all: build

build:
	mkdir -p build
	cd build && cmake ../
	cmake --build build

debug:
	mkdir -p build
	export Debug=true
	cd build && cmake ../
	cmake --build build