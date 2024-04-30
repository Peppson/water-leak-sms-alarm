
#pragma once
#include "config.h"
#include <type_traits>


class Memory {
public:
    Memory() = default;

    void begin();
    void end();
    static bool get_has_eeprom_failed();
    static void increment_eeprom_count(int address, int amount = 1); 
    static void reset_eeprom_count(int address);
    static void get_eeprom_counter_String(String& str, const int address);
    
    template <typename Type>
    static Type get_eeprom_count(int address) {
        static_assert(std::is_integral_v<Type> || std::is_same_v<Type, String>,
            "Type must be Arduino String or integral");

        if (_has_eeprom_failed) {
            return Type(0); 
        } else if (address == MemAddr::SmsSent) {
            return Type(EEPROM.read(address) + 1);
        } else {
            return Type(EEPROM.read(address)); 
        }
    }

private:
    static bool _has_eeprom_failed;
};
