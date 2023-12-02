.PHONY: all debug build

all: build

build:
	cmake -S. -Bbuild
	cmake --build build

debug:
	export Debug=true
	cmake -S. -Bbuild
	cmake --build build

clean:
	rm build -R