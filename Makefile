all: demo_static

.PHONY: clean
clean:
	rm *.o *.so demo_static

APU_LIB_SRCS:=nes_apu/Nes_Apu.cpp nes_apu/Blip_Buffer.cpp nes_apu/Nes_Oscs.cpp nes_apu/apu_snapshot.cpp Wave_Writer.cpp Simple_Apu.cpp Sound_Queue.cpp

# statically link the demo binary
demo_static: $(APU_LIB_SRCS) demo.cpp
	$(CXX) $(shell pkg-config -cflags sdl) $^ $(shell pkg-config -libs sdl) -o demo_static 
