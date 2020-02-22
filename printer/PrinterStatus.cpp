#include "PrinterStatus.h"
#include "../exceptions/PrinterError.h"

#include <iostream>
#include <iomanip>

using std::cout, std::endl;

const std::map<ErrorType, std::string> PrinterStatus::error_type_codes {
        {ErrorType::NO_ERROR, "No error"},
        {ErrorType::NO_MEDIA_WHEN_PRINTING, "No media when printing"},
        {ErrorType::END_OF_MEDIA, "End of media"},
        {ErrorType::TAPE_CUTTER_JAM, "Tape cutter jam"},
        {ErrorType::TRANSMISSION_ERROR, "Transmission error"},
        {ErrorType::COVER_OPENED_WHILE_PRINTING, "Cover opened while printing"},
        {ErrorType::CANNOT_FEED, "Cannot feed"},
        {ErrorType::SYSTEM_ERROR, "System error"}
};

const std::map<MediaType, std::string> PrinterStatus::media_type_codes {
        {MediaType::NO_MEDIA, "No media"},
        {MediaType::CONTINUOUS_LENGTH_TAPE, "Continuous length tape"},
        {MediaType::DIE_CUT_LABELS, "Die-cut labels"}
};

const std::map<StatusType, std::string> PrinterStatus::status_type_codes {
        {StatusType::REPLY_TO_STATUS_REQUEST, "Reply to status request"},
        {StatusType::PRINTING_COMPLETED, "Printing completed"},
        {StatusType::ERROR_OCCURRED, "Error occurred"},
        {StatusType::NOTIFICATION, "Notification"},
        {StatusType::PHASE_CHANGE, "Phase change"}
};

const std::map<PhaseType, std::string> PrinterStatus::phase_type_codes {
        {PhaseType::WAITING_TO_RECEIVE, "Waiting to receive"},
        {PhaseType::PRINTING_STATE, "Printing state"}
};

const std::map<NotificationType, std::string> PrinterStatus::notification_type_codes {
        {NotificationType::NOT_AVAILABLE, "Not available"},
        {NotificationType::COOLING_START, "Cooling (start)"},
        {NotificationType::COOLING_FINISH, "Cooling (finish)"}
};

PrinterStatus::PrinterStatus(const std::array<uint8_t, 32> &printer_status) noexcept
    : error_code { static_cast<uint16_t>((printer_status[ERROR_TYPE_OFFSET] << 8u) + printer_status[ERROR_TYPE_OFFSET+1]) },
    media_code { printer_status[MEDIA_TYPE_OFFSET] },
    status_code { printer_status[STATUS_TYPE_OFFSET] },
    phase_code { printer_status[PHASE_TYPE_OFFSET] },
    notification_code { printer_status[NOTIFICATION_TYPE_OFFSET] },
    media_length { printer_status[MEDIA_LENGTH_OFFSET] },
    media_width { printer_status[MEDIA_WIDTH_OFFSET] } {}

void PrinterStatus::display() const noexcept {
    cout << endl << "--- Printer Status ---" << endl;
    cout << std::left << std::setfill(' ');

    // Error
    const auto& err_it = error_type_codes.find((ErrorType) error_code);
    cout << std::setw(20) << "Error:" << (err_it == error_type_codes.end() ? "UNKNOWN" : err_it->second) << endl;

    // Media type
    const auto& med_it = media_type_codes.find((MediaType) media_code);
    cout << std::setw(20) << "Media type:" << (med_it == media_type_codes.end() ? "UNKNOWN" : med_it->second) << endl;

    // Status
    const auto& st_it = status_type_codes.find((StatusType) status_code);
    cout << std::setw(20) << "Status:" << (st_it == status_type_codes.end() ? "UNKNOWN" : st_it->second) << endl;

    // Phase
    const auto& ph_it = phase_type_codes.find((PhaseType) phase_code);
    cout << std::setw(20) << "Phase:" << (ph_it == phase_type_codes.end() ? "UNKNOWN" : ph_it->second) << endl;

    // Notification
    const auto& not_it = notification_type_codes.find((NotificationType) notification_code);
    cout << std::setw(20) << "Notification:" << (not_it == notification_type_codes.end() ? "UNKNOWN" : not_it->second) << endl;

    // Media dimensions
    cout << std::setw(20) << "Media dimensions:" << "[" << (unsigned) media_width << " x " << (unsigned) media_length << "]" << endl << endl;
}

bool PrinterStatus::check_error() const noexcept {
    return (ErrorType) error_code != ErrorType::NO_ERROR;
}

void PrinterStatus::check_error_throw() const {
    const auto& err_it = error_type_codes.find((ErrorType) error_code);
    if(err_it != error_type_codes.end() && err_it->first != ErrorType::NO_ERROR)
        throw PrinterError(err_it->second);
}
