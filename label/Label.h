#ifndef LABEL_PRINTER_DRIVER_LABEL_H
#define LABEL_PRINTER_DRIVER_LABEL_H

#include <string>
#include <map>
#include <vector>
#include <cairo/cairo.h>

enum class ProductUsage {
    HEATING,
    DEFROSTING,
    STORAGE
};

enum class Align {
    LEFT,
    CENTER,
    RIGHT
};

enum class LabelType {
    DIE_CUT,
    CONTINUOUS_LENGTH,
    UNDEFINED
};

namespace LabelSubtypes {
    struct LabelDimensions {
        uint8_t width_mm;
        uint8_t height_mm;
        uint32_t width_pt;
        uint32_t height_pt;
    };

    enum class DieCut {
        DC_29x90
    };

    enum class ContinuousLength {

    };

    extern const std::map<DieCut, LabelDimensions> die_cut_dimensions;
    extern const std::map<ContinuousLength, LabelDimensions> continuous_length_dimensions;
}

class Label {
private:
    static constexpr int SEPARATOR_WIDTH = 3;
    static constexpr double X_MARGIN = 0.05;
    static constexpr double Y_MARGIN = 0.25;

    const std::string product_name;
    const std::string product_usage;
    const std::string start_date;
    const std::string ready_date;
    const std::string end_date;

    inline static void set_label_dimensions(const LabelSubtypes::LabelDimensions& dimensions) noexcept;

    static void print_text(cairo_t *cr, const std::string& text, bool calculate_size, Align align,
                           const std::pair<double, double>& bl_corner,
                           const std::pair<double, double>& tr_corner) noexcept;

    [[nodiscard]] cairo_surface_t* create_label_surface() const noexcept;
    std::vector<uint8_t> prepare_for_printing(cairo_surface_t *surface) const noexcept;
    inline static bool thresh(const unsigned char *pix, int threshold = 190) noexcept;

public:
    static LabelType LABEL_TYPE;
    static int LABEL_WIDTH;
    static int LABEL_HEIGHT;
    static int LABEL_WIDTH_MM;
    static int LABEL_HEIGHT_MM;
    static const std::string FONT;

    static const std::map<ProductUsage, std::string> product_usage_texts;
    static const std::string start_date_text;
    static const std::string ready_date_text;
    static const std::string end_date_text;

    Label(std::string product, ProductUsage usage, std::string start, std::string ready, std::string end) noexcept;

    static void set_die_cut_label_type(LabelSubtypes::DieCut label_type, bool high_quality = false) noexcept;
    static void set_continuous_length_label_type(LabelSubtypes::ContinuousLength label_type, bool high_quality = false) noexcept;

    [[nodiscard]] std::vector<uint8_t> get_printing_data() const noexcept;
};


#endif //LABEL_PRINTER_DRIVER_LABEL_H
