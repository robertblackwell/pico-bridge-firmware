#ifndef PICO_ROBOT_FIRMWARE_HW_UTILS_H
#define PICO_ROBOT_FIRMWARE_HW_UTILS_H
#include <cstdint>
#include <cstdio>

struct HwUtils {
    static void pico_reset() {
#if PICO_ON_DEVICE
        *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
#else
        printf("pico_reset NOT pico_on_device\n");
#endif
    }
};

#endif //PICO_ROBOT_FIRMWARE_HW_UTILS_H
