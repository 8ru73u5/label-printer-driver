#include "PrinterError.h"
#include <iostream>

PrinterError::PrinterError(std::string error) noexcept : error(std::move(error)) {
    std::cerr << "Printer error: " << this->error << std::endl;
}
