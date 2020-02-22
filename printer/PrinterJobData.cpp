#include "PrinterJobData.h"

PrinterJobData::PrinterJobData(const Label &label) noexcept {
    label_type = Label::LABEL_TYPE;
    label_width = Label::LABEL_WIDTH_MM;
    label_height = Label::LABEL_HEIGHT_MM;
    raster_number = Label::LABEL_WIDTH;

    starting_page = true;
    auto_cut = false;
    cut_every_x_labels = 0;
    cut_at_end = true;
    high_quality = false;
    print_600dpi = false;
    margin_amount = 0;
}

void PrinterJobData::set_is_starting_page(const bool _starting_page) noexcept {
    starting_page = _starting_page;
}

void PrinterJobData::set_cut_options(const bool _auto_cut, const uint8_t _cut_every_x_labels, const bool _cut_at_end) noexcept {
    if(!_auto_cut && _cut_every_x_labels != 0)
        throw std::invalid_argument("Cut every X labels option is only available when auto cut is set to true");

    auto_cut = _auto_cut;
    cut_every_x_labels = _cut_every_x_labels;
    cut_at_end = _cut_at_end;
}

void PrinterJobData::set_quality(const bool _high_quality, const bool _print_600dpi) noexcept {
    high_quality = _high_quality;
    print_600dpi = _print_600dpi;
}

void PrinterJobData::set_margin_amount(uint16_t _margin_amount) noexcept {
    if(label_type == LabelType::DIE_CUT && _margin_amount != 0)
        throw std::invalid_argument("Die cut labels can only have margin amount of zero");

    if(_margin_amount != 35 && label_type == LabelType::CONTINUOUS_LENGTH)
        throw std::invalid_argument("Support only for QL-700 so margin for continuous labels can only be 35 dots");

    if(_margin_amount >= label_width)
        throw std::invalid_argument("Margin amount cannot be greater than the label's width");

    margin_amount = _margin_amount;
}

std::vector<uint8_t> PrinterJobData::construct_job_data_message() const noexcept {
    std::vector<uint8_t> job_data {};

    /* Set print information */
    std::array<uint8_t, 13> set_print_information {
        0x1b, 0x69, 0x7a,
        static_cast<uint8_t>((high_quality ? 0xce : 0x8e)),
        static_cast<uint8_t>((label_type == LabelType::DIE_CUT ? 0x0b : 0x0a)),
        label_height, label_width,
        static_cast<uint8_t>(raster_number & 0xffu),
        static_cast<uint8_t>((raster_number >> 8u) & 0xffu),
        static_cast<uint8_t>((raster_number >> 16u) & 0xffu),
        static_cast<uint8_t>((raster_number >> 24u) & 0xffu),
        static_cast<uint8_t>(starting_page),
        0x00
    };
    job_data.insert(job_data.end(), set_print_information.begin(), set_print_information.end());

    /* Set auto cut */
    std::array<uint8_t, 4> set_auto_cut_mode {
        0x1b, 0x69, 0x4d,
        static_cast<uint8_t>(auto_cut ? 0x40 : 0x00)
    };
    job_data.insert(job_data.end(), set_auto_cut_mode.begin(), set_auto_cut_mode.end());

    /* Set cut every x labels option */
    if(cut_every_x_labels != 0) {
        std::array<uint8_t, 4> set_cut_every_opt{
            0x1b, 0x69, 0x41,
            cut_every_x_labels
        };
        job_data.insert(job_data.end(), set_cut_every_opt.begin(), set_cut_every_opt.end());
    }

    /* Set expanded mode (cut at end and 600dpi printing) */
    std::array<uint8_t, 4> set_exp_mode {
        0x1b, 0x69, 0x4b,
        static_cast<uint8_t>((cut_at_end << 4u) + (print_600dpi << 6u))
    };
    job_data.insert(job_data.end(), set_exp_mode.begin(), set_exp_mode.end());

    /* Set margin amount */
    std::array<uint8_t, 5> set_margin {
        0x1b, 0x69, 0x64,
        static_cast<uint8_t>((margin_amount >> 8u) & 0xffu),
        static_cast<uint8_t>(margin_amount & 0xffu)
    };
    job_data.insert(job_data.end(), set_margin.begin(), set_margin.end());

    return job_data;
}
