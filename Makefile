.PHONY: libapu.so
CC=gcc
CXX=g++ -std=c++17
all: demo_static demo_shared libapu.so libapu.so

.PHONY: clean
clean:
	rm -f *.o *.so demo_static

APU_LIB_SRCS:=nes_apu/Nes_Apu.cpp nes_apu/Blip_Buffer.cpp nes_apu/Nes_Oscs.cpp nes_apu/apu_snapshot.cpp Simple_Apu.cpp
DEMO_SRCS:=Sound_Queue.cpp Wave_Writer.cpp demo.cpp

# statically link the demo binary
demo_static: $(APU_LIB_SRCS) $(DEMO_SRCS)
	$(CXX) $(shell pkg-config -cflags sdl) $^ $(shell pkg-config -libs sdl) -o demo_static 

# run the shared library
.PHONY: run_shared
run_shared: demo_shared
	LD_LIBRARY_PATH=$(shell pwd) ./demo_shared

# build the demo linked against the shared library
demo_shared: libapu.so $(DEMO_SRCS)
	$(CXX) $(shell pkg-config -cflags sdl) $^ libapu.so $(shell pkg-config -libs sdl) -o demo_shared

# CFLAGS=-fvisibility=hidden -ffreestanding -nostdlib -fPIC -O3 -Wfatal-errors
CFLAGS=-fvisibility=hidden -ffreestanding -fPIC -O3 -Wfatal-errors
libapu.so: $(APU_LIB_SRCS) libapu.cc
	$(CXX) -I. $(CFLAGS) -fPIC $^ -shared -o libapu.so

