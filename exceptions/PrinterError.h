#ifndef LABEL_PRINTER_DRIVER_PRINTERERROR_H
#define LABEL_PRINTER_DRIVER_PRINTERERROR_H

#include <string>

class PrinterError : std::exception {
public:
    const std::string error;

    explicit PrinterError(std::string error) noexcept;
};


#endif //LABEL_PRINTER_DRIVER_PRINTERERROR_H
