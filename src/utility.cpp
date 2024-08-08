
#include "utility.h"
#include "core/hardware.h"

namespace util {    

// Timer function. Call before and after measured code
void IRAM_ATTR benchmark(const char* name) {
    static bool begin_timer = true;
    static uint start_time = 0;

    // Start timer
	if (begin_timer) {
        begin_timer = false;
		start_time = micros();
        return;
	}

    // Stop timer + output
    uint end_time = micros() - start_time;
    String num_str = String(end_time);

    // Name (if any)
    if (name != nullptr) { log("%s: ", name); }
    
    // Microseconds
    if (num_str.length() < 4) {
        log("%iμs \n", end_time);

    // Milliseconds
    } else if (num_str.length() < 6) {
        float end_ms = (float)end_time / 1000;
        log("%.3fms \n", end_ms);
    } else {
        log("%ims \n", end_time / 1000);
    }

    // Reset
    begin_timer = true;
    start_time = 0;	
}

//
// Debug
//

void ESP32_print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0 : log("Wakeup caused by external signal using RTC_IO \n"); break;
        case ESP_SLEEP_WAKEUP_EXT1 : log("Wakeup caused by external signal using RTC_CNTL \n"); break;
        case ESP_SLEEP_WAKEUP_TIMER : log("Wakeup caused by timer \n"); break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD : log("Wakeup caused by touchpad \n"); break;
        case ESP_SLEEP_WAKEUP_ULP : log("Wakeup caused by ULP program \n"); break;
        default : break;
    }
}


void debug_loop(Memory& memory, GsmModule& sms) {
    hardware::led_color(Color::Off);
    uint32_t timeout = millis() + 5*1000;
    uint32_t current_time = 0;

    log("---- debug_loop() ----\n");
    while (true) {
        if (Serial.available()) {
            debug_USB_serial(memory, sms);
        }

        // Sometimes helps establish serial connection
        #if DEBUG_LOOP_PING
            if (current_time > timeout) {
                log(".\n"); 
                timeout = millis() + 5*1000;
            }
            current_time = millis();
        #endif 
    }
}


void debug_input_gpio_digital(int gpio_num) {
    while (1) {
        if (digitalRead(gpio_num)) {
            hardware::led_color(Color::Green);
        } else {
            hardware::led_color(Color::Off);
        }
        delay(1);
    }
}


void debug_input_gpio_analog(int gpio_num) {
    uint value = 0;
    while (1) {
        for (size_t i = 0; i < 25; i++) {
            value += analogRead(gpio_num);
            delay(1);
        }
        uint average = value / 25;
        (average > 1) ? hardware::led_color(Color::Green) : hardware::led_color(Color::Off);
        log("%i\n", average);
        value = 0;
    }
}


void debug_USB_serial(Memory& memory, GsmModule& sms) {
    char RX_serial = Serial.read();
    switch (RX_serial) {
        // Send SMS
        case 'a' : log("> sms.send_message(Alert)\n"); sms.send_message(SMSType::Alert);                        break; 
        case 's' : log("> sms.send_message(Diagnostic)\n"); sms.send_message(SMSType::Diagnostic);              break;

        // EEPROM SmsSent 
        case 'z': log("> SmsSent ++ \n"); memory.increment_eeprom_count(MemAddr::SmsSent);                      break;            
        case 'x': log("> SmsSent: %i \n", memory.get_eeprom_count<uint8_t>(MemAddr::SmsSent));                  break;      
        case 'c': log("> SmsSent reset back to 0 \n"); memory.reset_eeprom_count(MemAddr::SmsSent);             break;

        // EEPROM BootCount
        case '1': log("> BootCount ++ \n"); memory.increment_eeprom_count(MemAddr::BootCount);                  break;            
        case '2': log("> BootCount: %i \n", memory.get_eeprom_count<uint8_t>(MemAddr::BootCount));              break;      
        case '3': log("> BootCount reset back to 0 \n"); memory.reset_eeprom_count(MemAddr::BootCount);         break;

        // Misc
        case '4': log(">\n");                                                                                   break;
        case '5': log("> Deepsleep! \n"); delay(1000); hardware::deepsleep(10);                         	break;
        case '6': log("> Rebooting\n"); delay(1000); ESP.restart();                                             break;
        case '7': log("> sms.begin()\n"); sms.begin();                                                          break;
        case '8': log("> GSM power off\n"); sms.power_off();                                                    break;
        case '9': log("> ALL power off!\n"); 
            digitalWrite(PIN_CIRCUIT_POWER_SWITCH, HIGH); delay(1000);
            log("Resetting\n"); digitalWrite(PIN_CIRCUIT_POWER_SWITCH, LOW);               
            break;

        default: 
            break;
    }
}
} // Namespace util
