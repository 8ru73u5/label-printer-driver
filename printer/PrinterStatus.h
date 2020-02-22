#ifndef LABEL_PRINTER_DRIVER_PRINTERSTATUS_H
#define LABEL_PRINTER_DRIVER_PRINTERSTATUS_H

#include <cstdint>
#include <map>


enum class ErrorType : uint16_t {
    NO_ERROR = 0x0000,
    NO_MEDIA_WHEN_PRINTING = 0x0100,
    END_OF_MEDIA = 0x0200,
    TAPE_CUTTER_JAM = 0x0400,
    TRANSMISSION_ERROR = 0x0004,
    COVER_OPENED_WHILE_PRINTING = 0x0010,
    CANNOT_FEED = 0x0040,
    SYSTEM_ERROR = 0x0080
};

enum class MediaType : uint8_t {
    NO_MEDIA = 0x00,
    CONTINUOUS_LENGTH_TAPE = 0x0a,
    DIE_CUT_LABELS = 0x0b
};

enum class StatusType : uint8_t {
    REPLY_TO_STATUS_REQUEST = 0x00,
    PRINTING_COMPLETED = 0x01,
    ERROR_OCCURRED = 0x02,
    NOTIFICATION = 0x05,
    PHASE_CHANGE = 0x06
};

enum class PhaseType : uint8_t {
    WAITING_TO_RECEIVE = 0x00,
    PRINTING_STATE = 0x01
};

enum class NotificationType : uint8_t {
    NOT_AVAILABLE = 0x00,
    COOLING_START = 0x03,
    COOLING_FINISH = 0x04
};

struct PrinterStatus {
    static const std::map<ErrorType, std::string> error_type_codes;
    static const std::map<MediaType, std::string> media_type_codes;
    static const std::map<StatusType, std::string> status_type_codes;
    static const std::map<PhaseType, std::string> phase_type_codes;
    static const std::map<NotificationType, std::string> notification_type_codes;

    static constexpr uint8_t ERROR_TYPE_OFFSET = 8;
    static constexpr uint8_t STATUS_TYPE_OFFSET = 18;
    static constexpr uint8_t MEDIA_TYPE_OFFSET = 11;
    static constexpr uint8_t PHASE_TYPE_OFFSET = 19;
    static constexpr uint8_t NOTIFICATION_TYPE_OFFSET = 22;

    static constexpr uint8_t MEDIA_WIDTH_OFFSET = 10;
    static constexpr uint8_t MEDIA_LENGTH_OFFSET = 17;

    const uint16_t error_code;
    const uint8_t media_code;
    const uint8_t status_code;
    const uint8_t phase_code;
    const uint8_t notification_code;

    const uint8_t media_length;
    const uint8_t media_width;

    explicit PrinterStatus(const std::array<uint8_t, 32>& printer_status) noexcept;

    void display() const noexcept;
    [[nodiscard]] inline bool check_error() const noexcept;
    void check_error_throw() const;
};


#endif //LABEL_PRINTER_DRIVER_PRINTERSTATUS_H
