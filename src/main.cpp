
#include "config.h"
#include "core/gsm_module.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "utility.h"

using namespace hardware;
Memory memory;
GsmModule sms;

// Main
void setup() {
    begin_hardware(&memory, &sms);

    // Diagnostic test button
    if (is_test_button_pressed()) {
        sms.send_message(SMSType::Diagnostic);
        system_shutdown();
    }

    // Did we wake up from deepsleep? (reboots itself after deepsleep)
    if (woke_up_from_deepsleep()) {
        system_shutdown();
    }

    // Normal boot
    memory.increment_eeprom_count(MemAddr::BootCount, 1, false);  

    if (is_water_leak_detected()) {
        sms.send_message(SMSType::Alert);
        deepsleep(DEEPSLEEP_DURATION_LONG);  // Deepsleep to prevent retriggering
        /*OFF*/
    } else {
        deepsleep(DEEPSLEEP_DURATION_SHORT);  // False positive
        /*OFF*/
    }

    // Debug/development
    #if USB_SERIAL_ENABLED && DEBUG_LOOP_ENABLED
        util::debug_loop(memory, sms);
    #endif
}


void loop() {
    system_shutdown(); // Should never get here
} 
