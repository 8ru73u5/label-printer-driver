#include "PrinterJobData.h"

PrinterJobData::PrinterJobData() noexcept {
    if(!Label::is_valid())
        throw std::runtime_error("Label type not set! Use provided static function to set it");

    label_type = Label::get_type();
    const LabelDimensions dimensions = Label::get_dimensions();
    label_width = label_type == LabelType::DIE_CUT ? dimensions.width_mm : 0;
    label_height = dimensions.height_mm;
    raster_number = dimensions.width_pt;

    starting_page = true;
    auto_cut = false;
    cut_every_x_labels = 0;
    cut_at_end = true;
    high_quality = false;
    print_600dpi = false;
    margin_amount = label_type == LabelType::DIE_CUT ? 0 : 35;
}

void PrinterJobData::set_is_starting_page(const bool _starting_page) noexcept {
    starting_page = _starting_page;
}

void PrinterJobData::set_auto_cut_options(const bool _auto_cut, const uint8_t _cut_every_x_labels) noexcept {
    if(!_auto_cut && _cut_every_x_labels != 0)
        throw std::invalid_argument("Cut every X labels option is only available when auto cut is set to true");

    auto_cut = _auto_cut;
    cut_every_x_labels = _cut_every_x_labels;
}

void PrinterJobData::set_cut_at_end(const bool _cut_at_end) noexcept {
    cut_at_end = _cut_at_end;
}

void PrinterJobData::set_quality(const bool _high_quality) noexcept {
    high_quality = _high_quality;
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
        static_cast<uint8_t>(starting_page ? 0x00 : 0x01),
        0x00
    };
    job_data.insert(job_data.end(), set_print_information.begin(), set_print_information.end());
    job_data.insert(job_data.end(), {0x00, 0x01, static_cast<unsigned char>(high_quality ? 0xce : 0x8e)});

    /* Set auto cut */
    std::array<uint8_t, 4> set_auto_cut_mode {
        0x1b, 0x69, 0x4d,
        static_cast<uint8_t>(auto_cut ? 0x40 : 0x00)
    };
    job_data.insert(job_data.end(), set_auto_cut_mode.begin(), set_auto_cut_mode.end());

    /* Set cut every x labels option */
    if(auto_cut && cut_every_x_labels != 0) {
        std::array<uint8_t, 4> set_cut_every_opt{
            0x1b, 0x69, 0x41,
            cut_every_x_labels
        };
        job_data.insert(job_data.end(), set_cut_every_opt.begin(), set_cut_every_opt.end());
    }

    /* Set expanded mode (cut at end and 600dpi printing) */
    std::array<uint8_t, 4> set_exp_mode {
        0x1b, 0x69, 0x4b,
        static_cast<uint8_t>(static_cast<uint8_t>(cut_at_end << 3u) | static_cast<uint8_t>(print_600dpi << 6u))
    };
    job_data.insert(job_data.end(), set_exp_mode.begin(), set_exp_mode.end());

    /* Set margin amount */
    std::array<uint8_t, 5> set_margin {
        0x1b, 0x69, 0x64,
        static_cast<uint8_t>(margin_amount & 0xff),
        static_cast<uint8_t>((margin_amount >> 8u) & 0xff)
    };
    job_data.insert(job_data.end(), set_margin.begin(), set_margin.end());

    return job_data;
}
