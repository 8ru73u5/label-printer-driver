#include "USBError.h"
#include <iostream>
#include <utility>

USBError::USBError(std::string error, std::string where) noexcept : where(std::move(where)), error(std::move(error))  {
    std::cerr << "Libusb error: " << this->error << " -- Where: " << this->where << std::endl;
}
