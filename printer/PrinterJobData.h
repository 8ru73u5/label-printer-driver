#ifndef LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H
#define LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H

#include <vector>
#include <cinttypes>
#include "../label/Label.h"

class PrinterJobData {
private:
    LabelType label_type;
    uint8_t label_width;
    uint8_t label_height;
    uint32_t raster_number;

    bool starting_page;

    bool auto_cut;
    uint8_t cut_every_x_labels;
    bool cut_at_end;

    bool high_quality;
    bool print_600dpi;

    uint16_t margin_amount;

public:
    explicit PrinterJobData(const Label& label) noexcept;

    void set_is_starting_page(bool starting_page) noexcept;
    void set_cut_options(bool auto_cut, uint8_t cut_every_x_labels, bool cut_at_end) noexcept;
    void set_quality(bool high_quality, bool print_600dpi) noexcept;
    void set_margin_amount(uint16_t margin_amount) noexcept;

    [[nodiscard]] std::vector<uint8_t> construct_job_data_message() const noexcept;
};


#endif //LABEL_PRINTER_DRIVER_PRINTERJOBDATA_H
