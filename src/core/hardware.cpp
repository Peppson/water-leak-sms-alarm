
#include "core/hardware.h"
#include "utility.h"
#include <HardwareSerial.h>

namespace hardware {
    
static NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> pixel(1, PIN_LED); // Led
static Memory* _memory_ptr;
static GsmModule* _sms_ptr;


void IRAM_ATTR begin_hardware(Memory* memory, GsmModule* sms) {
    // Whole circuit power switch
    pinMode(PIN_CIRCUIT_POWER_SWITCH, OUTPUT);
    digitalWrite(PIN_CIRCUIT_POWER_SWITCH, LOW);

    // SIM800L power switch
    pinMode(PIN_SIM800L_POWER_SWITCH, OUTPUT);
    digitalWrite(PIN_SIM800L_POWER_SWITCH, LOW);

    pinMode(PIN_TEST_BUTTON, INPUT_PULLDOWN);

    // Memory, Serial and Led
    memory->begin(); 
    begin_USB_serial();
    pixel.Begin();    

    // Store pointers
    _memory_ptr = memory;
    _sms_ptr = sms;

    #if RESET_ALL
        _memory_ptr->reset_eeprom_count(MemAddr::BootCount); 
        _memory_ptr->reset_eeprom_count(MemAddr::SmsSent);
        #if USB_SERIAL_ENABLED
            log("\n\n---- Reset all counters ----\n     Awaiting upload...\n");
        #else 
            Serial.begin(115200);
            while (!Serial) { }
            Serial.printf("\n\n---- Reset all counters ----\n     Awaiting upload...\n");
        #endif
        while (1) { }
    #endif
}


void begin_USB_serial() {
    #if USB_SERIAL_ENABLED 
        Serial.begin(115200);
        while (!Serial) { delay(1); }

        // Print
        constexpr const char* emote = "\xF0\x9F\x98\x8E\xF0\x9F\x98\x8E\xF0\x9F\x98\x8E";
        log("\n\n\n\n");
        log("---- Initializing ---- %s \n", emote);
        log("---- %i Phone number(s)\n", GsmModule::NUM_OF_PHONES_TO_SMS);
        log("---- SEND_SMS_ENABLED = %s\n", SEND_SMS_ENABLED ? "true" : "false");
        util::ESP32_print_wakeup_reason();
    #endif
}


bool is_test_button_pressed() {
    bool pressed = digitalRead(PIN_TEST_BUTTON);

    if (pressed) { log("Test button pressed!\n"); }
    return pressed;
}


bool is_water_leak_detected() {
    pinMode(PIN_WATERLEAK_DETECT, INPUT_PULLDOWN);
    constexpr uint8_t times = 25;
    uint32_t value = 0;
    
    // Get average ADC reading
    for (uint8_t i = 0; i < times; i++) {
        value += analogRead(PIN_WATERLEAK_DETECT);
        delay(1);
    }

    pinMode(PIN_WATERLEAK_DETECT, OUTPUT);
    return (value / times > 5);
}


bool IRAM_ATTR woke_up_from_deepsleep() {
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    return (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER);
}


void deepsleep(const uint32_t& sleep_duration_seconds) {
    // Skip the normal deepsleep while debugging
    #if USB_SERIAL_ENABLED && DEBUG_LOOP_ENABLED
        return;
    #endif
    
    peripherals_shutdown();
    log("Deepsleep: %s\n", (sleep_duration_seconds < 60) ? "Short" : "Long");

    // Set wakeup sources: Timer and TEST_BUTTON
    esp_sleep_enable_timer_wakeup(sleep_duration_seconds * DEEPSLEEP_uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
    static_assert(PIN_TEST_BUTTON == 13, "TEST_BUTTON GPIO changed!");

    // ZzzzZZZzzZZZz
    delay(1000); // Needed to prevent bootlooping
    esp_deep_sleep_start();
    /*reboot*/
}


void peripherals_shutdown() {
    // GsmModule
    if (_sms_ptr) { 
        _sms_ptr->flush_buffers();
        _sms_ptr->power_off(); 
    }
    // EEPROM memory
    if (_memory_ptr) { 
        _memory_ptr->end(); 
    }
    // Led 
    led_end();
}


void system_shutdown() {
    peripherals_shutdown();

    // Circuit power latch OFF
    log("Power OFF!\n");
    digitalWrite(PIN_CIRCUIT_POWER_SWITCH, HIGH);  
    while (1) { }
    /* OFF */
}


void set_led_color(SMSType sms_type) {
    switch (sms_type) {
        case SMSType::Alert:
            hardware::led_color(Color::Orange); 
            break;
        case SMSType::Diagnostic:
            hardware::led_color(Color::Blue); 
            break;
        default:
            break;
    }
    delay(1);
}


void led_color(Color color) {
    const int led_index = 0; // Only 1 led in total
    
    // Set led color (color can be "Off")
    RgbColor RGB = map_color_to_RGB(color);
    pixel.SetPixelColor(led_index, RGB);
    pixel.Show();
}


void led_blink(size_t times, Color color, uint delay_ms) {
    for (size_t i = 0; i < times; i++) {
        led_color(color);       // On
        delay(delay_ms);
        led_color(Color::Off);  // Off
        delay(delay_ms);
    }
}


void led_end() {
    led_color(Color::Off);
    delay(3); // Needed
    pixel.~NeoPixelBus();
}


RgbColor map_color_to_RGB(Color color) {
    switch (color) {
        case Color::Orange:
            return RgbColor(175, 35, 0);
        case Color::Green:
            return RgbColor(0, 155, 0);
        case Color::Blue:
            return RgbColor(0, 0, 175);
        case Color::Red:
            return RgbColor(155, 0, 0);
        case Color::Off:
            return RgbColor(0, 0, 0);
        default:
            return RgbColor(0, 0, 0);
    }
}


void error() {
    log("Error! \n");
    led_blink(4, Color::Red, 250);

    #if DEBUG_LOOP_ENABLED
        return;
    #else
        system_shutdown();
    #endif
}
}; // Namespace hardware
