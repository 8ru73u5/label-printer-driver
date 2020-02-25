#include <cmath>
#include <cstring>
#include <iostream>
#include <utility>
#include "Label.h"
#include "../printer/Printer.h"
#include "LabelCreator.h"

LabelType Label::label_type = LabelType::UNDEFINED;
LabelDimensions Label::dimensions = {};

Label::Label(std::string product, ProductUsage usage, std::string start, std::optional<std::string> ready, std::string end) noexcept
        : product_name(std::move(product)),
        product_usage(usage),
        start_date(std::move(start)),
        ready_date(std::move(ready)),
        end_date(std::move(end)) {

    if(Label::label_type == LabelType::UNDEFINED)
        throw std::runtime_error("Label type not set! Use provided static function to set it");
};

Label::Label(std::string product, ProductUsage usage, std::string start, std::string end) noexcept
        : Label(std::move(product), usage, std::move(start), std::nullopt, std::move(end)) {}

Label Label::from_yaml_node(const YAML::Node &node, const ProductUsage usage) {
    const auto name = node["name"].as<std::string>();

    std::string usage_str {};
    switch(usage) {
        case ProductUsage::BOARD:   usage_str = "board";   break;
        case ProductUsage::PREP:    usage_str = "prep";    break;
        case ProductUsage::STORAGE: usage_str = "storage"; break;
    }

    std::optional<std::string> ready;
    if(node[usage_str]["ready"])
        ready = node[usage_str]["ready"].as<std::string>();
    else
        ready = std::nullopt;

    const auto discard = node[usage_str]["discard"].as<std::string>();

    return Label(name, usage, "", ready, discard);
}

void Label::set_die_cut_label_type(LabelSubtypes::DieCut _label_type, const bool high_quality) noexcept {
    const auto& lab_it = LabelSubtypes::die_cut_dimensions.find(_label_type);
    if(lab_it == LabelSubtypes::die_cut_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    Label::label_type = LabelType::DIE_CUT;
    Label::dimensions = lab_it->second;

    if(high_quality)
        Label::dimensions.width_pt *= 2;
}

void Label::set_continuous_length_label_type(LabelSubtypes::ContinuousLength _label_type, const int width_mm, const bool high_quality) noexcept {
    const auto& lab_it = LabelSubtypes::continuous_length_dimensions.find(_label_type);
    if(lab_it == LabelSubtypes::continuous_length_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    Label::label_type = LabelType::CONTINUOUS_LENGTH;
    Label::dimensions.height_mm = lab_it->second.height_mm;
    Label::dimensions.width_mm = width_mm;
    Label::dimensions.width_pt = static_cast<uint32_t>(round(width_mm * 0.03937 * 300));

    if(high_quality)
        Label::dimensions.width_pt *= 2;
}

std::vector<uint8_t> Label::prepare_for_printing(cairo_surface_t *surface) const noexcept {
    const size_t data_size = Label::dimensions.width_pt * 93;
    std::vector<uint8_t> printing_data(data_size, 0x00);
    size_t idx = 0;

    const unsigned char *label_data = cairo_image_surface_get_data(surface);
    const int stride = cairo_image_surface_get_stride(surface);
    const auto fill_amount = static_cast<size_t>(90 - std::ceil(Label::dimensions.height_pt / 8.0));  // Amount of zero-padding for each column
    const bool unaligned_pixels = Label::dimensions.height_pt % 8 != 0;

    uint8_t pixel_octet = 0;
    uint8_t pixels_packed = 7;
    for(int i = 0, offset = 0; i < Label::dimensions.width_pt; ++i, offset += 4) {
        // Add print command
        std::copy(Printer::print_data_cmd.begin(), Printer::print_data_cmd.end(), printing_data.begin() + idx);
        idx += 3;  // print_data_cmd.size()

        for(int k = 0; k < Label::dimensions.height_pt; ++k) {
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
    cairo_surface_t *label_surface = LabelCreator::create_label_surface(*this);
    std::vector<uint8_t> printing_data = prepare_for_printing(label_surface);
    cairo_surface_destroy(label_surface);

    return printing_data;
}

std::vector<Label> Label::load_label_definitions(const std::string& def_file) {
    const YAML::Node products = YAML::LoadFile(def_file)["products"];

    std::vector<Label> labels {};
    for(const auto& i: products) {
        if(i["board"])
            labels.push_back(Label::from_yaml_node(i, ProductUsage::BOARD));
        if(i["prep"])
            labels.push_back(Label::from_yaml_node(i, ProductUsage::PREP));
        if(i["storage"])
            labels.push_back(Label::from_yaml_node(i, ProductUsage::STORAGE));
    }

    return labels;
}
