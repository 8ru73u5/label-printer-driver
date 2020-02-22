cmake_minimum_required(VERSION 3.15)
project(label_printer_driver)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(label_printer_driver main.cpp)
add_library(label_printer_driver_libs printer/Printer.cpp exceptions/USBError.cpp printer/PrinterStatus.cpp exceptions/PrinterError.cpp label/Label.cpp printer/PrinterJobData.cpp)
target_link_libraries(label_printer_driver label_printer_driver_libs usb-1.0 cairo)