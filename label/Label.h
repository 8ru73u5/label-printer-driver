#ifndef LABEL_PRINTER_DRIVER_LABEL_H
#define LABEL_PRINTER_DRIVER_LABEL_H

#include <string>
#include <map>
#include <vector>
#include <cairo/cairo.h>

enum class ProductUsage {
    BOARD,
    PREP,
    STORAGE
};

enum class LabelType {
    DIE_CUT,
    CONTINUOUS_LENGTH,
    UNDEFINED
};

struct LabelDimensions {
    uint8_t width_mm;
    uint8_t height_mm;
    uint32_t width_pt;
    uint32_t height_pt;
};

namespace LabelSubtypes {
    enum class DieCut {
        DC_29x90
    };

    enum class ContinuousLength {

    };

    const std::map<DieCut, LabelDimensions> die_cut_dimensions {
            {LabelSubtypes::DieCut::DC_29x90, {90, 29, 991, 306}}
    };

    const std::map<ContinuousLength, LabelDimensions> continuous_length_dimensions {

    };
}

class Label {
private:
    static LabelType label_type;
    static LabelDimensions dimensions;

    const std::string product_name;
    const ProductUsage product_usage;
    const std::string start_date;
    const std::optional<std::string> ready_date;
    const std::string end_date;

    std::vector<uint8_t> prepare_for_printing(cairo_surface_t *surface) const noexcept;
    inline static bool thresh(const unsigned char *pix, int threshold = 190) noexcept;

public:
    Label(std::string product, ProductUsage usage, std::string start, std::optional<std::string> ready, std::string end) noexcept;
    Label(std::string product, ProductUsage usage, std::string start, std::string end) noexcept;

    static void set_die_cut_label_type(LabelSubtypes::DieCut _label_type, bool high_quality = false) noexcept;
    static void set_continuous_length_label_type(LabelSubtypes::ContinuousLength _label_type, bool high_quality = false) noexcept;

    [[nodiscard]] std::vector<uint8_t> get_printing_data() const noexcept;

    friend class LabelCreator;
    friend class PrinterJobData;
};


#endif //LABEL_PRINTER_DRIVER_LABEL_H
