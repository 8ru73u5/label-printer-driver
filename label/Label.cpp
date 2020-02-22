#include <cmath>
#include <cstring>
#include <iostream>
#include "Label.h"
#include "../printer/Printer.h"

const std::map<LabelSubtypes::DieCut, LabelSubtypes::LabelDimensions> LabelSubtypes::die_cut_dimensions {
        {LabelSubtypes::DieCut::DC_29x90, {90, 29, 991, 306}}
};

const std::map<LabelSubtypes::ContinuousLength, LabelSubtypes::LabelDimensions> LabelSubtypes::continuous_length_dimensions {

};

LabelType Label::LABEL_TYPE = LabelType::UNDEFINED;
int Label::LABEL_WIDTH = -1;
int Label::LABEL_HEIGHT = -1;
int Label::LABEL_WIDTH_MM = -1;
int Label::LABEL_HEIGHT_MM = -1;
const std::string Label::FONT = "Noto Sans";

const std::map<ProductUsage, std::string> Label::product_usage_texts {
        {ProductUsage::HEATING,    "ogrzewanie"    },
        {ProductUsage::DEFROSTING, "rozmrażanie"   },
        {ProductUsage::STORAGE,    "przechowywanie"}
};

const std::string Label::start_date_text = "Data wyjęcia:";
const std::string Label::ready_date_text = "Data gotowości:";
const std::string Label::end_date_text   = "Data przydatności:";

Label::Label(std::string product, ProductUsage usage, std::string start, std::string ready, std::string end) noexcept
        : product_name(std::move(product)),
        product_usage { product_usage_texts.at(usage) },
        start_date(std::move(start)),
        ready_date(std::move(ready)),
        end_date(std::move(end)) {

    if(product_usage_texts.find(usage) == product_usage_texts.end())
        throw std::runtime_error("product_usage_texts map does not contain this usage key");

    if(LABEL_TYPE == LabelType::UNDEFINED)
        throw std::runtime_error("Label type not set! Use provided static function to set it");
};

inline void Label::set_label_dimensions(const LabelSubtypes::LabelDimensions& dimensions) noexcept {
    LABEL_WIDTH = dimensions.width_pt;
    LABEL_HEIGHT = dimensions.height_pt;
    LABEL_WIDTH_MM = dimensions.width_mm;
    LABEL_HEIGHT_MM = dimensions.height_mm;
}

void Label::set_die_cut_label_type(LabelSubtypes::DieCut label_type, const bool high_quality) noexcept {
    const auto& lab_it = LabelSubtypes::die_cut_dimensions.find(label_type);
    if(lab_it == LabelSubtypes::die_cut_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    LABEL_TYPE = LabelType::DIE_CUT;
    set_label_dimensions(lab_it->second);

    if(high_quality)
        LABEL_WIDTH *= 2;
}

void Label::set_continuous_length_label_type(LabelSubtypes::ContinuousLength label_type, const bool high_quality) noexcept {
    const auto& lab_it = LabelSubtypes::continuous_length_dimensions.find(label_type);
    if(lab_it == LabelSubtypes::continuous_length_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    LABEL_TYPE = LabelType::CONTINUOUS_LENGTH;
    set_label_dimensions(lab_it->second);

    if(high_quality)
        LABEL_WIDTH *= 2;
}

cairo_surface_t *Label::create_label_surface() const noexcept {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, LABEL_WIDTH, LABEL_HEIGHT);
    cairo_t *cr = cairo_create(surface);

    /* Draw background */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    /* Draw separators */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, SEPARATOR_WIDTH);
    cairo_move_to(cr, 0, LABEL_HEIGHT * 0.1);
    cairo_line_to(cr, LABEL_WIDTH, LABEL_HEIGHT * 0.1);
    cairo_move_to(cr, 0, LABEL_HEIGHT * 0.4);
    cairo_line_to(cr, LABEL_WIDTH, LABEL_HEIGHT * 0.4);
    cairo_move_to(cr, 0, LABEL_HEIGHT * 0.7);
    cairo_line_to(cr, LABEL_WIDTH, LABEL_HEIGHT * 0.7);
    cairo_move_to(cr, LABEL_WIDTH * 0.5, LABEL_HEIGHT * 0.1);
    cairo_line_to(cr, LABEL_WIDTH * 0.5, LABEL_HEIGHT);
    cairo_stroke(cr);

    /* Draw product name */
    cairo_select_font_face(cr, FONT.c_str(), CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);
    print_text(cr, product_name + " (" + product_usage + ")", true, Align::CENTER,
            std::pair(0, LABEL_HEIGHT * 0.1),
            std::pair(LABEL_WIDTH, 0));

    /* Draw date texts */
    cairo_select_font_face(cr, FONT.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    print_text(cr, end_date_text, true, Align::RIGHT,
            std::pair(0, LABEL_HEIGHT),
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT * 0.7));
    print_text(cr, ready_date_text, false, Align::RIGHT,
            std::pair(0, LABEL_HEIGHT * 0.7),
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT * 0.4));
    print_text(cr, start_date_text, false, Align::RIGHT,
            std::pair(0, LABEL_HEIGHT * 0.4),
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT * 0.1));

    /* Draw dates */
    print_text(cr, start_date, false, Align::CENTER,
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT * 0.4),
            std::pair(LABEL_WIDTH, LABEL_HEIGHT * 0.1));
    print_text(cr, ready_date, false, Align::CENTER,
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT * 0.7),
            std::pair(LABEL_WIDTH, LABEL_HEIGHT * 0.4));
    print_text(cr, end_date, false, Align::CENTER,
            std::pair(LABEL_WIDTH / 2, LABEL_HEIGHT),
            std::pair(LABEL_WIDTH, LABEL_HEIGHT * 0.7));

    cairo_surface_flush(surface);
    cairo_destroy(cr);

    return surface;
}

void Label::print_text(cairo_t *cr, const std::string &text, bool calculate_size, const Align align,
                       const std::pair<double, double> &bl_corner,
                       const std::pair<double, double> &tr_corner) noexcept {

    const double max_height = bl_corner.second - tr_corner.second;
    const double max_width = tr_corner.first - bl_corner.first;

    cairo_text_extents_t ext;
    if(calculate_size) {
        double font_size = max_height * (1 - Y_MARGIN);
        do {
            cairo_set_font_size(cr, font_size--);
            cairo_text_extents(cr, text.c_str(), &ext);
        } while(ext.width > max_width * (1 - X_MARGIN));
    }
    else
        cairo_text_extents(cr, text.c_str(), &ext);

    double text_x;
    if(align == Align::RIGHT)
        text_x = tr_corner.first - ext.x_advance - max_width * (X_MARGIN / 2);
    else if(align == Align::CENTER)
        text_x = bl_corner.first + (max_width - ext.width) / 2;
    else
        throw std::runtime_error("Only Align::RIGHT and Align::CENTER are supported for now");

    double text_y = bl_corner.second - (ext.height + ext.y_bearing) - (max_height - ext.height) / 2;

    cairo_move_to(cr, text_x, text_y);
    cairo_show_text(cr, text.c_str());
}

std::vector<uint8_t> Label::prepare_for_printing(cairo_surface_t *surface) const noexcept {
    const size_t data_size = LABEL_WIDTH * 93;
    std::vector<uint8_t> printing_data(data_size, 0x00);
    size_t idx = 0;

    const unsigned char *label_data = cairo_image_surface_get_data(surface);
    const int stride = cairo_image_surface_get_stride(surface);
    const auto fill_amount = static_cast<size_t>(90 - std::ceil(LABEL_HEIGHT / 8.0));  // Amount of zero-padding for each column
    const bool unaligned_pixels = LABEL_HEIGHT % 8 != 0;

    uint8_t pixel_octet = 0;
    uint8_t pixels_packed = 7;
    for(int i = 0, offset = 0; i < LABEL_WIDTH; ++i, offset += 4) {
        // Add print command
        std::copy(Printer::print_data_cmd.begin(), Printer::print_data_cmd.end(), printing_data.begin() + idx);
        idx += 3;  // print_data_cmd.size()

        for(int k = 0; k < LABEL_HEIGHT; ++k) {
            bool pixel = thresh(label_data + (k * stride + offset));
            pixel_octet |= static_cast<uint8_t>(pixel << pixels_packed);  // Set pixel value

            if(pixels_packed == 0) {
                printing_data[idx++] = pixel_octet;
                pixel_octet = 0;
                pixels_packed = 7;
            }
            else
                --pixels_packed;
        }

        if(unaligned_pixels) {
            printing_data[idx++] = pixel_octet;
            pixel_octet = 0;
            pixels_packed = 7;
        }

        idx += fill_amount;
    }

    return printing_data;
}

bool Label::thresh(const unsigned char *pix, int threshold) noexcept {
    return (pix[0] * 0.299 + pix[1] * 0.587 + pix[2] * 0.114) < threshold;
}

std::vector<uint8_t> Label::get_printing_data() const noexcept {
    cairo_surface_t *label_surface = create_label_surface();
    std::vector<uint8_t> printing_data = prepare_for_printing(label_surface);
    cairo_surface_destroy(label_surface);

    return printing_data;
}
