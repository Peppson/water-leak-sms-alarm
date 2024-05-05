
#pragma once
#include <Arduino.h>
#include <stdbool.h>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <EEPROM.h>


// Dev
#define SEND_SMS_ENABLED 0                                          // SMS toggle 
#define USB_SERIAL_ENABLED 0                                        // USB serial connection toggle
#define DEBUG_LOOP_ENABLED 0                                        // Enter debug loop
#define DEBUG_LOOP_PING 0                                           // Serial print (".") while in debug_loop, sometimes helps with connection     
#define RESET_ALL 0                                                 // Resets all counters and await new code upload   

// Setup
constexpr const char* SMS_ALERT_ROW_0 = "WARNING!";                 // Alert sms first row
constexpr const char* SMS_ALERT_ROW_1 = "Water leak detected!";     // Alert sms second row
constexpr const char* SMS_DIAGNOSTIC_ROW_0 = "Status!";             // Diagnostic sms first row
constexpr uint32_t CONNECTION_TIMEOUT = 2*60*1000;                  // Wait for GSM network connection (mS)
constexpr uint32_t DEEPSLEEP_DURATION_LONG = 1*60*60;               // Deepsleep after waterleak is detected (S)
constexpr uint32_t DEEPSLEEP_DURATION_SHORT = 10;                   // Deepsleep after false positive (S)
constexpr uint16_t SERIAL_RESPONSE_TIMEOUT = 75;                    // Wait for serial response from SIM800L (mS)
constexpr uint16_t MAX_SMS_UNTILL_EMPTY_SIMCARD = 30;               // How many sms can we send in total? (money/sms cost)
constexpr uint64_t DEEPSLEEP_uS_TO_S_FACTOR = 1000000;              // Factor

// Physical I/O
#define PIN_CIRCUIT_POWER_SWITCH 19                                 // Pull HIGH to turn off ALL power (latching circuit)
#define PIN_SIM800L_POWER_SWITCH 26                                 // Turn on/off power to the SIM800L module 
#define PIN_TEST_BUTTON 13                                          // Input push button
#define PIN_WATERLEAK_DETECT 4                                      // Input (third conductive leg) to detect water leakage
#define PIN_LED 23                                                  // Datapin WS2812B led (SPI MOSI)
#define PIN_SIM800L_RX 16                                           // ESP32 hardware serial 2 RX
#define PIN_SIM800L_TX 17                                           // ESP32 hardware serial 2 TX

// Print toggle
#if USB_SERIAL_ENABLED
    #define log(...) Serial.printf(__VA_ARGS__)
    #define STOP log("\n-----  Stop right there criminal scum!  ----- \n"); while (1) { delay(1); }
#else
    #define log(...)
    #define STOP
#endif

// Enums
enum class SMSType : uint8_t {
    Alert,
    Diagnostic,
    None
};

enum class Color : uint8_t {
    Orange,
    Green,
    Blue,
    Red,
    Off
};

enum MemAddr : int {
    BootCount,
    SmsSent,
    NumOfMemAddr  // Num of items. Well, it only grew to two
};
