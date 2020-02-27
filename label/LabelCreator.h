#ifndef LABEL_PRINTER_DRIVER_LABELCREATOR_H
#define LABEL_PRINTER_DRIVER_LABELCREATOR_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cairo/cairo.h>

#include "Label.h"
#include "ProductLabel.h"

namespace {
    struct Point {
        double x;
        double y;
    };

    struct Guide {
        Point start;
        Point end;
    };

    enum class Align {
        LEFT, CENTER, RIGHT
    };

    const std::map<std::string, Align> __aligns {
            {"left", Align::LEFT},
            {"center", Align::CENTER},
            {"right", Align::RIGHT}
    };

    enum class Binding {
        PRODUCT_NAME,
        START_DATE_TEXT,
        READY_DATE_TEXT,
        DISCARD_DATE_TEXT,
        START_DATE,
        READY_DATE,
        DISCARD_DATE
    };

    const std::map<std::string, Binding> __bindings {
            {"product_name", Binding::PRODUCT_NAME},
            {"start_date_text", Binding::START_DATE_TEXT},
            {"ready_date_text", Binding::READY_DATE_TEXT},
            {"discard_date_text", Binding::DISCARD_DATE_TEXT},
            {"start_date", Binding::START_DATE},
            {"ready_date", Binding::READY_DATE},
            {"discard_date", Binding::DISCARD_DATE}
    };

    struct Font {
        std::string face;
        cairo_font_slant_t slant;
        cairo_font_weight_t weight;
    };

    const std::map<std::string, cairo_font_slant_t> __slants {
            {"normal", CAIRO_FONT_SLANT_NORMAL},
            {"italic", CAIRO_FONT_SLANT_ITALIC},
            {"oblique", CAIRO_FONT_SLANT_OBLIQUE}
    };

    const std::map<std::string, cairo_font_weight_t> __weights {
            {"normal", CAIRO_FONT_WEIGHT_NORMAL},
            {"bold", CAIRO_FONT_WEIGHT_BOLD}
    };

    struct TextBox {
        std::optional<Font> font;
        Align align {};
        Point bottom_left {};
        Point top_right {};
    };
}

class LabelCreator {
private:
    Font global_font;

    std::string start_date_text;
    std::string ready_date_text;
    std::string discard_date_text;

    std::string date_format;

    std::string usage_board_text;
    std::string usage_prep_text;
    std::string usage_storage_text;

    double guide_width {};
    double text_box_margin_x {};
    double text_box_margin_y {};

    std::vector<Guide> guides;
    std::map<Binding, TextBox> text_boxes;

    std::string config_file;

    void calculate_font_size(cairo_t *cr, const std::string& text, Binding bind);
    void print_text(cairo_t *cr, const std::string& text, Binding bind);

    static std::chrono::hours detect_duration(const std::string& date);
    static std::string date_to_str(const std::chrono::system_clock::time_point& date);


    LabelCreator() = default;
    static LabelCreator _inst;

public:
    static void load_config(const std::string& config_file);
    inline static void reload_config();

    [[nodiscard]] static cairo_surface_t *create_label_surface(const ProductLabel& label);
    static void export_to_png(const ProductLabel& label, const std::string& filename);
};


#endif //LABEL_PRINTER_DRIVER_LABELCREATOR_H
