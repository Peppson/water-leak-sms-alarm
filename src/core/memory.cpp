
#include "core/memory.h"

bool Memory::_has_eeprom_failed; 


void Memory::begin() {
    constexpr int mem_size = static_cast<int>(MemAddr::NumOfMemAddr);

    // Begin EEPROM
    if (!EEPROM.begin(mem_size)) {
        _has_eeprom_failed = true;
        return;
    }

    // Init value on first boot
    for (int address = 0; address < mem_size; address++) {
        if (EEPROM.read(address) == 255) {
            reset_eeprom_count(address);
        }
    }
}


void Memory::increment_eeprom_count(int address, int amount) {
    if (_has_eeprom_failed) { return; }

    // Incremeent
    uint8_t value = EEPROM.read(address);
    EEPROM.write(address, value + amount);

    // Failure
    if (!EEPROM.commit()) {
        log("Failed to write into EEPROM! \n");
        _has_eeprom_failed = true;
    }
}


void Memory::reset_eeprom_count(int address) {
    if (_has_eeprom_failed) { return; }

    // Reset memory bank
    EEPROM.write(address, 0);

    // Failure
    if (!EEPROM.commit()) {
        log("Failed to reset in EEPROM! \n");
        _has_eeprom_failed = true;
    }
}


void Memory::get_eeprom_counter_String(String& str, const int address) {
    // Check if EEPROM has failed  
    if (get_has_eeprom_failed()) {
        str = "EEPROM failed!";

    // BootCount
    } else if (address == MemAddr::BootCount) {
        str = get_eeprom_count<String>(address) + " (resets to 0)";
        reset_eeprom_count(address);

    // SmsSent
    } else if (address == MemAddr::SmsSent) {
        str = get_eeprom_count<String>(address) + "/" + MAX_SMS_UNTILL_EMPTY_SIMCARD;
    }
}


bool Memory::get_has_eeprom_failed() {
    return _has_eeprom_failed;
}


void Memory::end() {
    EEPROM.end(); // void
}
