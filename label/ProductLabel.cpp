#include "ProductLabel.h"
#include "LabelCreator.h"

ProductLabel::ProductLabel(std::string _name, ProductUsage _usage, std::optional<std::time_t> start,
        std::optional<std::string> ready, std::string discard) noexcept
    : name(std::move(_name)),
    usage(_usage),
    start_date(start),
    ready_date(std::move(ready)),
    discard_date(std::move(discard)) {}

ProductLabel::ProductLabel(const YAML::Node& node, const ProductUsage _usage) {
    usage = _usage;
    std::string usage_str {};
    switch(usage){
        case ProductUsage::BOARD:   usage_str = "board";   break;
        case ProductUsage::PREP:    usage_str = "prep";    break;
        case ProductUsage::STORAGE: usage_str = "storage"; break;
        default: throw std::runtime_error("Attempt to use unimplemented ProductUsage");
    }

    if(!node["name"])
        throw std::runtime_error("You haven't specified 'name' for some product in your config file.");
    name = node["name"].as<std::string>();

    if(!node[usage_str])
        throw std::runtime_error("You don't have '" + usage_str + "' for '" + name + "' product in your config file");
    ready_date = node[usage_str]["ready"] ?
            std::optional<std::string>(node[usage_str]["ready"].as<std::string>())
                    : std::nullopt;

    if(!node[usage_str]["discard"])
        throw std::runtime_error("You haven't specified 'discard' for '" + name + "' product in your config file");
    discard_date = node[usage_str]["discard"].as<std::string>();
}

inline void ProductLabel::set_start_date(std::optional<std::time_t> start) noexcept {
    start_date = start;
}

std::vector<uint8_t> ProductLabel::prepare_for_printing(cairo_surface_t *surface) const {
    const size_t data_size = Label::dimensions.width_pt * 93;
    std::vector<uint8_t> printing_data(data_size, 0x00);
    size_t idx = 0;

    const unsigned char *label_data = cairo_image_surface_get_data(surface);
    const int stride = cairo_image_surface_get_stride(surface);
    const auto fill_amount = static_cast<size_t>(90 - std::ceil(Label::dimensions.height_pt / 8.0));  // Amount of zero-padding for each column
    const bool unaligned_pixels = Label::dimensions.height_pt % 8 != 0;

    constexpr std::array<uint8_t, 3> print_data_cmd {0x67, 0x00, 0x5a};

    uint8_t pixel_octet = 0;
    uint8_t pixels_packed = 7;
    for(int i = 0, offset = 0; i < Label::dimensions.width_pt; ++i, offset += 4) {
        // Add print command
        std::copy(print_data_cmd.begin(), print_data_cmd.end(), printing_data.begin() + idx);
        idx += print_data_cmd.size();

        for(int k = 0; k < Label::dimensions.height_pt; ++k) {
            const bool pixel = thresh(label_data + (k * stride + offset));
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

inline bool ProductLabel::thresh(const unsigned char *pix, int threshold) noexcept {
    return (pix[0] * 0.299 + pix[1] * 0.587 + pix[2] * 0.114) < threshold;
}

std::vector<uint8_t> ProductLabel::get_printing_data() const {
    cairo_surface_t *label_surface = LabelCreator::create_label_surface(*this);
    std::vector<uint8_t> printing_data = prepare_for_printing(label_surface);
    cairo_surface_destroy(label_surface);

    return printing_data;
}

std::vector<Label*> ProductLabel::load_label_definitions(const std::string &def_file) {
    const YAML::Node products = YAML::LoadFile(def_file)["products"];

    std::vector<Label*> labels {};
    for(const auto& i: products) {
        if(i["board"])
            labels.push_back(new ProductLabel(i, ProductUsage::BOARD));
        if(i["prep"])
            labels.push_back(new ProductLabel(i, ProductUsage::PREP));
        if(i["storage"])
            labels.push_back(new ProductLabel(i, ProductUsage::STORAGE));
    }

    return labels;
}
