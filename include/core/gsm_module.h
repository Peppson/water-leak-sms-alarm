
#pragma once
#include "config.h"
#include "secrets.h"
#include <HardwareSerial.h>


class GsmModule {
public:
    GsmModule() : GSM_serial(2) {}  // Use UART2 bus
    HardwareSerial GSM_serial;      // Connection to SIM800L module: RX = gpio 16, TX = gpio 17

    void begin(const SMSType sms_type = SMSType::None);
    void send_message(const SMSType sms_type);
    void flush_buffers();
    void flush_RX_buffer();
    void power_off();

    // How many phone numbers are entered?
    constexpr static int NUM_OF_PHONES_TO_SMS = 
        sizeof(secret_phone_numbers) / sizeof(secret_phone_numbers[0]); 

private:
    // Static members
    static bool _is_sim800l_on;
    static int _signal_strength;
    static String _boot_counter;
    static String _total_sms_sent; 
    static std::string _model_name;
    static std::string _network_operator;

    // Methods
    bool send_sms_guard();
    bool send_sms(const SMSType sms_type, const char* phone_number);
    bool is_GSM_connected();
    void get_diagnostic_details();
    int get_GSM_signal_strength();
    void get_model_name(std::string& model_name);
    void get_network_operator(std::string& operator_name);
    bool get_network_operator_name(std::string& operator_name);
    void get_serial_response(std::string& response, bool print = false, uint extra_delay = 0);
    bool verify_serial_response(std::string& response);
    bool send_serial_and_verify(const char* command, bool print = false, uint extra_delay = 0);
    bool send_serial_and_verify(const char* command, std::string& response, bool print = false, uint extra_delay = 0);
    int string_to_int(std::string& response);
};
