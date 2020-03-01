#ifndef LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H
#define LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H

#include "../label/Label.h"

/**
 * Representation of a job data message that is sent
 * to the printer before printing of each page.
 */
class PrinterJobData {
private:
    LabelType label_type;
    uint8_t label_width;  /**< Width of the label in millimeters */
    uint8_t label_height;  /**< Height of the label in millimeters */
    uint32_t raster_number;  /**< Width of the label in pixels (points) */

    bool starting_page;

    std::optional<uint8_t> cut_every_x_labels;
    bool cut_at_end;

    bool high_quality;  /**< Prefer quality over speed of printing */

    uint16_t margin_amount;

public:

    /**
     * Default constructor that sets label properties according to
     * values of `Label::dimensions` and `Label::type` and the rest of
     * the class members is set to somewhat arbitrary default values.
     *
     * @throws std::runtime_error if `Label::is_valid()` returns `false`
     * @see Label
     */
    PrinterJobData();

    void set_is_starting_page(bool starting_page) noexcept;

    /**
     * Sets `cut_every_x_labels` member
     *
     * @param cut_every_x_labels Value in range (0, 255> or literally `{}`
     *
     * @throws std::invalid_argument if parameter is equal to zero
     */
    void set_auto_cut_options(std::optional<uint8_t> cut_every_x_labels);

    void set_cut_at_end(bool cut_at_end) noexcept;
    void set_quality(bool high_quality) noexcept;

    /**
     * Set `margin_amount` member.
     *
     * Parameter in millimeters is converted to pixels (points).
     *
     * @param margin_amount_mm Margin amount specified in millimeters
     *
     * @throws std::runtime_error if `label_type = LabelType::DIE_CUT`
     * @throws std::invalid_argument if the parameter is not in range <3, 127>
     */
    void set_margin_amount(uint8_t margin_amount_mm);

    /**
     * Constructs job data message from member values.
     *
     * @return Vector of bytes that represents job data packet
     */
    [[nodiscard]] std::vector<uint8_t> construct_job_data_message() const noexcept;
};


#endif //LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H
