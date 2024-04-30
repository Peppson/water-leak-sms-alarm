
#pragma once 
#include "config.h"
#include "core/memory.h"
#include "core/gsm_module.h"
#include <NeoPixelBus.h>


namespace hardware {        
    void IRAM_ATTR begin_hardware(Memory* memory, GsmModule* sms);
    void begin_USB_serial();
    bool is_test_button_pressed();
    bool is_water_leak_detected();
    bool IRAM_ATTR woke_up_from_deepsleep();
    void deepsleep(const uint32_t& sleep_duration_seconds);
    void peripherals_shutdown();
    void system_shutdown();
    void set_led_color(SMSType sms_type);
    void led_color(Color color);
    void led_blink(size_t times, Color color, uint delay_ms);
    void led_end();
    RgbColor map_color_to_RGB(Color color);
    void error();
};
