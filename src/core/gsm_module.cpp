
#include "core/gsm_module.h"
#include "core/hardware.h"
#include "utility.h"

//
// Private members
//

bool GsmModule::_is_sim800l_on = false;
int GsmModule::_signal_strength = 0;
String GsmModule::_boot_counter;
String GsmModule::_total_sms_sent; 
std::string GsmModule::_model_name = "undefined";
std::string GsmModule::_network_operator = "undefined";

//
// Public
//

bool GsmModule::begin(const SMSType sms_type) {
    hardware::set_led_color(sms_type);
    digitalWrite(PIN_SIM800L_POWER_SWITCH, HIGH); 

    // Establish UART connection
    delay(8000);
    GSM_serial.begin(9600, SERIAL_8N1, PIN_SIM800L_RX, PIN_SIM800L_TX);

    // Handshake
    if (!send_serial_and_verify("AT")) {
        log("SIM800L device not found! \nConfigured serial pins: Gpio %i(TX) Gpio %i(RX)\n", 
            PIN_SIM800L_TX, PIN_SIM800L_RX);

    // Is SIM card installed?
    } else if (!send_serial_and_verify("AT+CCID")) { 
        log("Simcard not found! \n");

    // Powered on!
    } else {
        log("SIM800L powered on! \n");
        _is_sim800l_on = true;
        return true;
    }

    return false;
}


void GsmModule::send_message(const SMSType sms_type) {
    uint32_t connection_timeout = millis() + CONNECTION_TIMEOUT;
    uint32_t current_time = 0;
    bool all_sent = true;

    // Initialize the SIM800L simcard module
    begin(sms_type);

    // Wait for connection 
    while (!is_GSM_connected() && connection_timeout > current_time) {
        delay(500);
        log(".");
        _is_sim800l_on ? current_time = millis() : begin();
    }
    get_diagnostic_details(sms_type);

    // Send SMS (to multiple or single number)
    for (const auto& phone_number : secret_phone_numbers) {
        if (!send_sms(sms_type, phone_number)) { 
            all_sent = false;
        }
    }

    // SMS successful?
    if (all_sent) {
        Memory::increment_eeprom_count(MemAddr::SmsSent, NUM_OF_PHONES_TO_SMS);
        hardware::led_blink(4, Color::Green, 250);
    } else {
        hardware::error();
    }
}


void GsmModule::flush_buffers() {
    GSM_serial.flush();
}


void GsmModule::flush_RX_buffer() {
    // Wait and capture message in the RX buffer
    delay(SERIAL_RESPONSE_TIMEOUT);

    // Empty the buffer, read into nothing
    GSM_serial.setTimeout(10);
    while (GSM_serial.available()) {
        GSM_serial.readString();
    }
}


void GsmModule::power_off() {
    log("SIM800L OFF!\n");
    digitalWrite(PIN_SIM800L_POWER_SWITCH, LOW);
}


//
// Private
//


bool GsmModule::send_sms_guard() {
    #if !SEND_SMS_ENABLED
        log("\nSEND_SMS_ENABLED = false\n");
        return false;
    #else
        static uint8_t sms_counter = 0;

        if (sms_counter < NUM_OF_PHONES_TO_SMS) {
            sms_counter++;
            log("\nSending SMS %i/%i\n", sms_counter, NUM_OF_PHONES_TO_SMS);
            return true;
        } else {
            log("Maximum number of SMS (%i/%i) already sent!\n", sms_counter, NUM_OF_PHONES_TO_SMS);
            return false;
        }
    #endif
}


bool GsmModule::send_sms(const SMSType sms_type, const char* phone_number) {
    if (!send_sms_guard()) { return false; }

    // SMS mode
    GSM_serial.println("AT+CMGF=1");    
    flush_RX_buffer(); // Needed! 4 hours debugging went into this :)))

    // Phone number
    GSM_serial.print("AT+CMGS=\"");        
    GSM_serial.print(phone_number);
    GSM_serial.println("\"");
    flush_RX_buffer();

    // SMS: Alert
    if (sms_type == SMSType::Alert) {
        GSM_serial.printf("%s\r\n", SMS_ALERT_ROW_0);
        GSM_serial.printf("%s\r\n", SMS_ALERT_ROW_1);

    // SMS: Diagnostic
    } else if (sms_type == SMSType::Diagnostic) {
        GSM_serial.printf("%s\r\n",                 SMS_DIAGNOSTIC_ROW_0);
        GSM_serial.printf("- Signal: %i%%\r\n",     _signal_strength);
        GSM_serial.printf("- Network: %s\r\n",      _network_operator.c_str());
        GSM_serial.printf("- Model: %s\r\n",        _model_name.c_str());
        GSM_serial.printf("- Sms sent: %s\r\n",     _total_sms_sent.c_str());
        GSM_serial.printf("- Boot count: %s\r\n",   _boot_counter.c_str());
    }

    // Send!
    GSM_serial.write(26);
    flush_RX_buffer();

    // Acknowledge
    std::string response = "";
    get_serial_response(response, false, 7000);
    return verify_serial_response(response);
}


bool GsmModule::is_GSM_connected() {    
    return static_cast<bool>(get_GSM_signal_strength());
}


void GsmModule::get_diagnostic_details(const SMSType sms_type) {
    if (sms_type == SMSType::Alert) { 
        return;
    }

    // Grab info  
    _signal_strength = get_GSM_signal_strength();
    get_model_name(_model_name);
    get_network_operator(_network_operator);
    Memory::get_eeprom_counter_String(_boot_counter, MemAddr::BootCount);
    Memory::get_eeprom_counter_String(_total_sms_sent, MemAddr::SmsSent);

    #if 0
        log("\n%s\r\n",                 SMS_DIAGNOSTIC_ROW_0);
        log("- Signal: %i%%\r\n",     _signal_strength);
        log("- Network: %s\r\n",      _network_operator.c_str());
        log("- Model: %s\r\n",        _model_name.c_str());
        log("- Sms sent: %s\r\n",     _total_sms_sent.c_str());
        log("- Boot count: %s\r\n",   _boot_counter.c_str());
        STOP
    #endif
}


int GsmModule::get_GSM_signal_strength() {
    std::string signal_strength = "";      

    // Expects "AT+CSQ\n+CSQ: XX,X\n\nOK"
    if (!send_serial_and_verify("AT+CSQ", signal_strength)) { 
        return 0; 
    }

    // Find index of "CSQ:"
    size_t index = signal_strength.find("CSQ:");

    // Not found
    if (index == std::string::npos) {
        return 0;
    }
    
    // Grab substring of numbers (CSQ: XX) 
    signal_strength = signal_strength.substr(index + 5, 2); 

    // Remove ',' if a single digit nummber (X,)
    if (signal_strength[1] == ',') { 
        signal_strength.pop_back(); 
    }

    // Max signal strength is 31. Convert to %
    int mapped_signal = string_to_int(signal_strength);
    return map(mapped_signal, 0, 31, 0, 100);
}


void GsmModule::get_model_name(std::string& model_name) {    
    // Expects "ATI\n SIMXXX RXX.XX\n\nOK"
    if (!send_serial_and_verify("ATI", model_name)) { 
        return; 
    }

    // Find index of "SIM"
    size_t index = model_name.find("SIM");

    // Not found
    if (index == std::string::npos) {
        return;
    }

    // Grab substring of model nr "SIMXXX RXX.XX" 
    model_name = model_name.substr(index, 13); 
}


void GsmModule::get_network_operator(std::string& operator_name) {
    uint32_t timeout = millis() + 10*1000;
    uint32_t current_time = 0;

    while (timeout > current_time) {
        if (get_network_operator_name(operator_name)) {
            break;
        } else {
            delay(500);
            log(".");
            current_time = millis();
        }
    }
}


bool GsmModule::get_network_operator_name(std::string& operator_name) {
    // Expects "+COPS?\n+COPS: 0,0,"{operator name}"\n\nOK"
    if (!send_serial_and_verify("AT+COPS?", operator_name)) { 
        return false; 
    }
    
    // Find start and end index of "{operator name}"
    size_t start = operator_name.find("\"") + 1;
    size_t end = operator_name.rfind("\"");

    // Not found
    if (end == std::string::npos) {
        return false;
    }

    // Grab substring of {operator name} 
    operator_name = operator_name.substr(start, end - start);
    return true;
}


void GsmModule::get_serial_response(std::string& response, bool print, uint extra_delay) {
    // Wait and capture message in the RX buffer
    delay(SERIAL_RESPONSE_TIMEOUT + extra_delay); 

    // Early return
    if (GSM_serial.available() <= 0) {
        if (print) { log("Early return! \n"); }
        return;
    }

    // Append incoming data
    GSM_serial.setTimeout(10);
    while (GSM_serial.available()) {
        response.append( GSM_serial.readString().c_str() );  
    }

    // Debug
    if (print) {
        log("\n\n%s\n\n", response.c_str());
    }
}


bool GsmModule::verify_serial_response(std::string& response) {
    // Return true if "OK" found in response
    return (response.rfind("OK") != std::string::npos) ? true : false;
}


bool GsmModule::send_serial_and_verify(const char* command, bool print, uint extra_delay) {
    std::string response = "";
    GSM_serial.println(command);                        // Send command      
    get_serial_response(response, print, extra_delay);  // Grab returning response    
               
    return verify_serial_response(response);            // Look for "OK" in response     
}


bool GsmModule::send_serial_and_verify(const char* command, std::string& response, bool print, uint extra_delay) {
    GSM_serial.println(command);                        // Send command      
    get_serial_response(response, print, extra_delay);  // Grab returning response    
               
    return verify_serial_response(response);            // Look for "OK" in response     
}


int GsmModule::string_to_int(std::string& response) {
    try {
        return std::stoi(response);

    } catch (const std::invalid_argument&) {
        log("Error in string_to_int() -> Invalid argument\n");
        return 0;

    } catch (const std::out_of_range&) {
        log("Error in string_to_int() -> Out of range\n");
        return 0;
    }
}
