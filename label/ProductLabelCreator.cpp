#include <algorithm>
#include <chrono>
#include <iomanip>
#include <yaml-cpp/yaml.h>

#include "ProductLabelCreator.h"

ProductLabelCreator ProductLabelCreator::_inst {};

void ProductLabelCreator::load_config(const std::string& _config_file) {
    _inst.config_file = _config_file;
    YAML::Node root = YAML::LoadFile(_inst.config_file);

    _inst.global_font = {
        root["global_font"]["face"].as<std::string>(),
        __slants.at(root["global_font"]["slant"].as<std::string>()),
        __weights.at(root["global_font"]["weight"].as<std::string>())
    };

    _inst.date_format = root["date_format"].as<std::string>();

    _inst.start_date_text = root["start_date_text"].as<std::string>();
    _inst.ready_date_text = root["ready_date_text"].as<std::string>();
    _inst.discard_date_text = root["discard_date_text"].as<std::string>();

    _inst.usage_board_text = root["usage_texts"]["board"].as<std::string>();
    _inst.usage_prep_text = root["usage_texts"]["prep"].as<std::string>();
    _inst.usage_storage_text = root["usage_texts"]["storage"].as<std::string>();

    _inst.guide_width = root["guide_width"].as<double>();
    _inst.text_box_margin_x = root["text_box_margins"]["x"].as<double>();
    _inst.text_box_margin_y = root["text_box_margins"]["y"].as<double>();

    for(const auto& guide: root["guides"]) {
        const Point start {guide["start"]["x"].as<double>(), guide["start"]["y"].as<double>()};
        const Point end {guide["end"]["x"].as<double>(), guide["end"]["y"].as<double>()};
        _inst.guides.push_back({start, end});
    }

    for(const auto& text_box: root["text_boxes"]) {
        const Binding bind_to = __bindings.at(text_box["bind_to"].as<std::string>());
        const Align align = __aligns.at(text_box["align"].as<std::string>());
        const Point bottom_left {text_box["bottom_left"]["x"].as<double>(), text_box["bottom_left"]["y"].as<double>()};
        const Point top_right {text_box["top_right"]["x"].as<double>(), text_box["top_right"]["y"].as<double>()};
        std::optional<Font> font;
        if(text_box["font"]) {
            font = {
                text_box["font"]["face"] ? text_box["font"]["face"].as<std::string>() : _inst.global_font.face,
                text_box["font"]["slant"] ? __slants.at(text_box["font"]["slant"].as<std::string>()) : _inst.global_font.slant,
                text_box["font"]["weight"] ? __weights.at(text_box["font"]["weight"].as<std::string>()) : _inst.global_font.weight
            };
        }
        else font = std::nullopt;

        _inst.text_boxes[bind_to] = {font, align, bottom_left, top_right};
    }
}

inline void ProductLabelCreator::reload_config() {
    if(_inst.config_file.empty())
        throw std::runtime_error("No config file to reload");
    ProductLabelCreator::load_config(_inst.config_file);
}

cairo_surface_t *ProductLabelCreator::create_label_surface(const ProductLabel& label) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, ProductLabel::dimensions.width_pt, ProductLabel::dimensions.height_pt);
    cairo_t *cr = cairo_create(surface);

    /* Draw background */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    /* Draw guides */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, _inst.guide_width);
    for(const auto& guide: _inst.guides) {
        cairo_move_to(cr, ProductLabel::dimensions.width_pt * guide.start.x, ProductLabel::dimensions.height_pt * guide.start.y);
        cairo_line_to(cr, ProductLabel::dimensions.width_pt * guide.end.x, ProductLabel::dimensions.height_pt * guide.end.y);
    }
    cairo_stroke(cr);

    /* Draw product name */
    cairo_select_font_face(cr, _inst.global_font.face.c_str(), _inst.global_font.slant, _inst.global_font.weight);
    std::string product_name = label.name + " (";
    switch(label.usage) {
        case ProductUsage::BOARD:   product_name += _inst.usage_board_text;   break;
        case ProductUsage::PREP:    product_name += _inst.usage_prep_text;    break;
        case ProductUsage::STORAGE: product_name += _inst.usage_storage_text; break;
    }
    product_name += ")";

    _inst.calculate_font_size(cr, product_name, Binding::PRODUCT_NAME);
    _inst.print_text(cr, product_name, Binding::PRODUCT_NAME);

    /* Draw date texts */
    std::vector<std::pair<std::string, Binding>> date_texts {
            {_inst.start_date_text, Binding::START_DATE_TEXT},
            {_inst.ready_date_text, Binding::READY_DATE_TEXT},
            {_inst.discard_date_text, Binding::DISCARD_DATE_TEXT}
    };

    // Find the longest date text and set font size for it
    std::sort(date_texts.begin(), date_texts.end(),
            [](const std::pair<std::string, Binding>& i, const std::pair<std::string, Binding>& k) {
                    return i.first.size() > k.first.size();
    });
    _inst.calculate_font_size(cr, date_texts[0].first, date_texts[0].second);

    // Remove ready_date_text if the label doesn't have ready_date
    if(!label.ready_date)
        date_texts.erase(date_texts.begin() + 1);

    for(const auto& i: date_texts)
        _inst.print_text(cr, i.first, i.second);

    /* Calculate dates */
    std::map<std::string, Binding> dates {};

    auto now = label.start_date ? std::chrono::system_clock::from_time_t(label.start_date.value()) : std::chrono::system_clock::now();
    decltype(now) ready;
    decltype(now) discard = now + detect_duration(label.discard_date);

    if(label.ready_date) {
        ready = now + detect_duration(label.ready_date.value());
        dates.insert({date_to_str(ready), Binding::READY_DATE});
    }

    dates.insert({date_to_str(now), Binding::START_DATE});
    dates.insert({date_to_str(discard), Binding::DISCARD_DATE});

    /* Draw dates */
    for(const auto& i: dates)
        _inst.print_text(cr, i.first, i.second);

    cairo_surface_flush(surface);
    cairo_destroy(cr);

    return surface;
}

void ProductLabelCreator::calculate_font_size(cairo_t *cr, const std::string &text, Binding bind) {
    const TextBox& text_box = text_boxes.at(bind);

    const double max_width = (text_box.top_right.x - text_box.bottom_left.x) * ProductLabel::dimensions.width_pt;
    const double max_height = (text_box.bottom_left.y - text_box.top_right.y) * ProductLabel::dimensions.height_pt;

    cairo_text_extents_t ext;
    double font_size = max_height * (1 - text_box_margin_y);
    do {
        cairo_set_font_size(cr, font_size--);
        cairo_text_extents(cr, text.c_str(), &ext);
    } while(ext.width > max_width * (1 - text_box_margin_x));
}

void ProductLabelCreator::print_text(cairo_t *cr, const std::string& text, const Binding bind) {
    const TextBox& text_box = text_boxes.at(bind);

    cairo_text_extents_t ext;
    cairo_text_extents(cr, text.c_str(), &ext);

    const double max_width = (text_box.top_right.x - text_box.bottom_left.x) * ProductLabel::dimensions.width_pt;
    const double max_height = (text_box.bottom_left.y - text_box.top_right.y) * ProductLabel::dimensions.height_pt;

    const double tr_x = text_box.top_right.x * ProductLabel::dimensions.width_pt;
    const double bl_x = text_box.bottom_left.x * ProductLabel::dimensions.width_pt;
    const double bl_y = text_box.bottom_left.y * ProductLabel::dimensions.height_pt;

    double text_x, text_y;

    switch(text_box.align) {
        case Align::LEFT:
            text_x = bl_x + max_width * (text_box_margin_x / 2);
            break;
        case Align::CENTER:
            text_x = bl_x + (max_width - ext.width) / 2;
            break;
        case Align::RIGHT:
            text_x = tr_x - ext.x_advance - max_width * (text_box_margin_x / 2);
            break;
    }

    text_y = bl_y - (ext.height + ext.y_bearing) - (max_height - ext.height) / 2;

    if(text_box.font)
        cairo_select_font_face(cr, text_box.font->face.c_str(), text_box.font->slant, text_box.font->weight);

    cairo_move_to(cr, text_x, text_y);
    cairo_show_text(cr, text.c_str());

    // Revert font face back to global
    if(text_box.font)
        cairo_select_font_face(cr, global_font.face.c_str(), global_font.slant, global_font.weight);
}

void ProductLabelCreator::export_to_png(const ProductLabel &label, const std::string& filename) {
    cairo_surface_t *surface = ProductLabelCreator::create_label_surface(label);
    cairo_surface_write_to_png(surface, filename.c_str());
    cairo_surface_destroy(surface);
}

std::chrono::hours ProductLabelCreator::detect_duration(const std::string &date) {
    const char interval_specifier = date[date.size() - 1];

    int interval;
    switch(interval_specifier) {
        case 'h': interval = std::stoi(date);      break;
        case 'd': interval = std::stoi(date) * 24; break;
        default:
            throw std::invalid_argument("Not implemented interval specifier");
    }

    return std::chrono::hours(interval);
}

std::string ProductLabelCreator::date_to_str(const std::chrono::system_clock::time_point& base) {
    std::stringstream buffer {};

    const std::time_t base_time = std::chrono::system_clock::to_time_t(base);
    const std::tm *t = std::localtime(&base_time);

    buffer << std::put_time(t, _inst.date_format.c_str());

    return buffer.str();
}
