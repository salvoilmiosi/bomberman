MAKE = make

ifeq ($(OS),Windows_NT)
	MAKE := mingw32-make
endif

all: client server

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean

client:
	$(MAKE) -C client
	
server:
	$(MAKE) -C server

.PHONY: clean client server