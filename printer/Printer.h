#ifndef LABEL_PRINTER_DRIVER_PRINTER_H
#define LABEL_PRINTER_DRIVER_PRINTER_H

#include "../exceptions/USBError.h"
#include "PrinterStatus.h"
#include "PrinterJobData.h"
#include <libusb-1.0/libusb.h>
#include <string>
#include <array>
#include <vector>

class Printer {
private:
    static constexpr uint16_t BROTHER_VID = 0x04f9;
    static constexpr uint16_t BROTHER_PID = 0x2042;

    static constexpr uint8_t BROTHER_ENDPOINT_IN = 0x02;
    static constexpr uint8_t BROTHER_ENDPOINT_OUT = 0x81;

    static constexpr uint8_t BROTHER_INTERFACE = 0x00;

    libusb_context *ctx = nullptr;
    libusb_device_handle *printer = nullptr;

    void cleanup() noexcept;
    inline void check_usb_error_throw(const int ret, const std::string& where, bool clean = true);
    inline static bool check_usb_error(const int ret) noexcept;
    inline static void libusb_error_to_stderr(const int error_code) noexcept;

    void send(std::vector<uint8_t>& data);
    PrinterStatus receive_status();
    void send_job_data(const PrinterJobData& job_data);
    void send_page_data(const Label& page_data, bool last_page);

public:
    Printer();
    ~Printer() noexcept;

    void scan_for_printer(uint8_t scan_timeout = 5);

    void clear_jobs();
    void init();

    PrinterStatus send_request_status();
    void print(const std::vector<Label*>& labels, PrinterJobData job_data);
};


#endif //LABEL_PRINTER_DRIVER_PRINTER_H
