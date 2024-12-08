// libapu exposes a simple C style interface for using the Simple_Apu
// interface.
// 
// This is easier to use when calling the library via shared library.
// libapu Copyright EmulationOnline
//
// Licensed under the LGPL version 2.1 license with the rest of this library.
// See LICENSE for more details.
//
#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include "Simple_Apu.h"
#include "nes_apu/apu_snapshot.h"

/*
 * Public Interface Declaration
 */
// Initialize with a given output sample_rate
extern "C" void libapu_init(int sample_rate);
// write to a mmio register
extern "C" void libapu_reg_write(uint16_t a, uint8_t v);
// read from status (0x4015)
extern "C" uint8_t libapu_read_status();
// produce as many samples as available
extern "C" long libapu_sample(int16_t* output, size_t capacity);
// indicate the end of a 1/60th second video frame
extern "C" void libapu_end_frame(void);
// Same and load support. Both assume the fd is seeked to the place
// to read or write apu state bytes.
// Returns 1 on success or 0 on failure.
extern "C" char libapu_load_state(int fd);
extern "C" char libapu_save_state(int fd);
// DMC support. DMC needs to read samples from memory. This
// tells the dmc how to read bytes from the system bus.
extern "C" void libapu_set_dmc_read(
	int (*callback)( void* user_data, unsigned ), void* user_data = NULL );


/*
 * Implementations
 */
Simple_Apu* _global_apu = 0;

extern "C"
__attribute__((visibility("default")))
void libapu_init(int sample_rate) {
    if (_global_apu != 0) {
        delete _global_apu;
    }

    _global_apu = new Simple_Apu();
    _global_apu->sample_rate(sample_rate);
}

extern "C"
__attribute__((visibility("default")))
void libapu_reg_write(uint16_t a, uint8_t v) {
    _global_apu->write_register(a, v);
}

extern "C"
__attribute__((visibility("default")))
uint8_t libapu_read_status() {
    return _global_apu->read_status();
}

extern "C"
__attribute__((visibility("default")))
long libapu_sample(int16_t* output, size_t capacity) {
    return _global_apu->read_samples(output, capacity);
}

extern "C"
__attribute__((visibility("default")))
void libapu_end_frame() {
    _global_apu->end_frame();
}

// Load/Save utils
// All values written in native byte order.
// ARM is usually little endian, as is x86. No other systems are
// currently considered.
// Dumps are assumed to be taken between frames.
// Returns 1 on success or 0 on failure.
int writeall(int fd, uint8_t *data, size_t len) {
    while(len > 0) {
        ssize_t w = write(fd, data, len);
        if (w  == -1) {
            perror("write failed.");
            return 0;
        }
        len -= w;
        data += len;
    }
    return 1;
}

int readall(int fd, uint8_t* dst, size_t len) {
    while (len > 0) {
        ssize_t r = read(fd, (void*)dst, len);
        if (r == -1) {
            perror("read failed.");
            return 0;
        } else if (r == 0) {
            perror("eof while writing");
            return 0;
        }

        len -= r;
        dst += r;
    }
    return 1;
}

extern "C"
__attribute__((visibility("default")))
char libapu_load_state(int fd) {
    apu_snapshot_t state;
    if (!readall(fd, (uint8_t*)&state, sizeof(apu_snapshot_t))) {
        puts("Failed to read apu state");
        return 0;
    }
    _global_apu->load_snapshot(state);
    return 1;
}

extern "C"
__attribute__((visibility("default")))
char libapu_save_state(int fd) {
    apu_snapshot_t state;
    _global_apu->save_snapshot(&state);
    if (!writeall(fd, (uint8_t*)&state, sizeof (apu_snapshot_t))) {
        puts("Failed to write apu state");
        return 0;
    }
    return 1;
}

extern "C"
__attribute__((visibility("default")))
void libapu_set_dmc_read(
	int (*callback)( void* user_data, unsigned ), void* user_data) {

    _global_apu->dmc_reader(
            (int(*)(void*, unsigned))callback, 
            user_data);
}
