#include "Printer.h"
#include "PrinterStatus.h"
#include <iostream>
#include <unistd.h>
#include <array>
#include <cstring>
#include <vector>

using std::cout, std::endl;

Printer::Printer() {
    cout << "Initializing libusb... ";
    check_usb_error_throw(libusb_init(&ctx), "constructor");
    cout << "done!" << endl;
}

Printer::~Printer() noexcept {
    cleanup();
}

void Printer::cleanup() noexcept {
    cout << "Performing cleanup for Printer..." << endl;

    if(printer != nullptr) {
        cout << " -> Releasing interface... ";
        if(!check_usb_error(libusb_release_interface(printer, BROTHER_INTERFACE)))
            cout << "done!" << endl;

        cout << " -> Closing printer handle... ";
        libusb_close(printer);
        cout << "done!" << endl;
    }

    cout << " -> Deinitializing libusb... ";
    libusb_exit(ctx);
    cout << "done!" << endl;

    cout << "Cleanup finished" << endl;
}

void Printer::libusb_error_to_stderr(const int error_code) noexcept {
    std::cerr << "libusb error: " << libusb_error_name(error_code) << endl;
}

bool Printer::check_usb_error(const int ret) noexcept {
    if(ret < LIBUSB_SUCCESS) {
        libusb_error_to_stderr(ret);
        return true;
    }
    return false;
}

void Printer::check_usb_error_throw(const int ret, const std::string& where, bool clean) {
    if(check_usb_error(ret)) {
        if(clean)
            cleanup();
        throw USBError(libusb_error_name(ret), where);
    }
}

void Printer::scan_for_printer(uint8_t scan_timeout) {
    libusb_device **device_list;
    bool found = false;

    cout << "Scanning for printer..." << endl;
    while(!found) {
        auto device_count = libusb_get_device_list(ctx, &device_list);
        check_usb_error_throw(device_count, "getting device list");

        for(auto i = 0; i < device_count; ++i) {
            libusb_device_descriptor desc {};
            check_usb_error_throw(libusb_get_device_descriptor(device_list[i], &desc), "getting device descriptor");

            if(desc.idVendor == BROTHER_VID && desc.idProduct == BROTHER_PID) {
                cout << "Printer found!" << endl;
                cout << " -> Opening device... ";
                check_usb_error_throw(libusb_open(device_list[i], &printer), "opening device");
                cout << "done!" << endl;

                cout << " -> Detaching kernel driver... ";
                check_usb_error_throw(libusb_set_auto_detach_kernel_driver(printer, 1), "setting auto detach kernel");
                cout << "done!" << endl;

                cout << " -> Claiming interface... ";
                check_usb_error_throw(libusb_claim_interface(printer, BROTHER_INTERFACE), "claiming interface");
                cout << "done!" << endl;

                found = true;
                break;
            }
        }

        libusb_free_device_list(device_list, 1);

        if(!found) {
            cout << "Printer not found! Retrying in " << scan_timeout << " seconds..." << endl;
            sleep(scan_timeout);
        }
    }

    cout << "Scan finished!" << endl;
}

void Printer::send(std::vector<uint8_t>& data) {
    int actual;
    check_usb_error_throw(
            libusb_bulk_transfer(printer, BROTHER_ENDPOINT_IN, data.data(), data.size(), &actual, 0),
            "sending data",
            false);

    cout << " [<< " << actual << "] ";
}

PrinterStatus Printer::receive_status() {
    constexpr size_t RECV_BUFFER_SIZE = 32;
    std::array<uint8_t, RECV_BUFFER_SIZE> buffer {};

    int actual;
    check_usb_error_throw(
            libusb_bulk_transfer(printer, BROTHER_ENDPOINT_OUT, buffer.data(), RECV_BUFFER_SIZE, &actual, 0),
            "receiving data",
            false
            );

    cout << " [>> " << actual << "] ";
    return PrinterStatus(buffer);
}

PrinterStatus Printer::send_request_status() {
    std::vector<uint8_t> request_status_cmd {0x1b, 0x69, 0x53};

    cout << "Requesting status information... ";
    send(request_status_cmd);
    cout << "done!" << endl;

    cout << "Waiting for response... ";
    PrinterStatus status = receive_status();
    cout << "done!" << endl;

    return status;
}

void Printer::clear_jobs() {
    std::vector<uint8_t> clear_jobs_cmd(200, 0x00);

    cout << "Clearing printer jobs... ";
    send(clear_jobs_cmd);
    cout << "done!" << endl;
}

void Printer::init() {
    std::vector<uint8_t> init_cmd {0x1b, 0x40};

    cout << "Initializing printer... ";
    send(init_cmd);
    cout << "done!" << endl;
}

void Printer::send_job_data(const PrinterJobData& job_data) {
    std::vector<uint8_t> raw_data(job_data.construct_job_data_message());

    cout << "Sending job data... ";
    send(raw_data);
    cout << "done!" << endl;
}

void Printer::send_page_data(const Label& label, const bool last_page) {
    std::vector<uint8_t> page_data = label.get_printing_data();
    page_data.push_back(last_page ? 0x1a : 0x0c);

    cout << "Sending page data... ";
    send(page_data);
    cout << "done!" << endl;
}

void Printer::print(const std::vector<Label*>& labels, PrinterJobData job_data) {
    clear_jobs();
    init();

    job_data.set_is_starting_page(true);
    for(size_t i = 0; i < labels.size(); ++i) {
        if(i == 1)
            job_data.set_is_starting_page(false);

        send_job_data(job_data);
        send_page_data(*labels[i], i == labels.size() - 1);

        PrinterStatus status = receive_status();
        status.display();
        PrinterStatus status_finished = receive_status();
        status_finished.display();
    }
}
