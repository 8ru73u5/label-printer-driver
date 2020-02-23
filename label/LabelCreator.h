#ifndef LABEL_PRINTER_DRIVER_LABELCREATOR_H
#define LABEL_PRINTER_DRIVER_LABELCREATOR_H

#include <string>
#include <vector>
#include <map>
#include <cairo/cairo.h>
#include "Label.h"

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
        END_DATE_TEXT,
        START_DATE,
        READY_DATE,
        END_DATE
    };

    const std::map<std::string, Binding> __bindings {
            {"product_name", Binding::PRODUCT_NAME},
            {"start_date_text", Binding::START_DATE_TEXT},
            {"ready_date_text", Binding::READY_DATE_TEXT},
            {"end_date_text", Binding::END_DATE_TEXT},
            {"start_date", Binding::START_DATE},
            {"ready_date", Binding::READY_DATE},
            {"end_date", Binding::END_DATE}
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
    std::string end_date_text;

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

    LabelCreator() = default;
    static LabelCreator _inst;

public:
    static void load_config(const std::string& config_file);
    inline static void reload_config();

    [[nodiscard]] static cairo_surface_t *create_label_surface(const Label& label);
    static void export_to_png(const Label& label, const std::string& filename);
};


#endif //LABEL_PRINTER_DRIVER_LABELCREATOR_H
