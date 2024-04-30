
#pragma once
#include "config.h"
#include "core/memory.h"
#include "core/gsm_module.h"


namespace util {
    void IRAM_ATTR benchmark(const char* name = nullptr);
    void ESP32_print_wakeup_reason();
    void debug_loop(Memory& memory, GsmModule& sms);
    void debug_input_gpio_digital(int gpio_num);
    void debug_input_gpio_analog(int gpio_num);
    void debug_USB_serial(Memory& memory, GsmModule& sms);
}
