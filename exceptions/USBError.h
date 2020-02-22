#ifndef LABEL_PRINTER_DRIVER_USBERROR_H
#define LABEL_PRINTER_DRIVER_USBERROR_H

#include "../printer/Printer.h"
#include <string>

class USBError : std::exception {
public:
    const std::string where;
    const std::string error;

    USBError(std::string error, std::string where) noexcept;
};


#endif //LABEL_PRINTER_DRIVER_USBERROR_H
