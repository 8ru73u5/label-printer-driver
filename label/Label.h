#ifndef LABEL_PRINTER_DRIVER_LABEL_H
#define LABEL_PRINTER_DRIVER_LABEL_H

#include <string>
#include <map>
#include <vector>
#include <cairo/cairo.h>
#include <yaml-cpp/yaml.h>

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
    enum class ContinuousLength : int {
        CL_12 = 257, CL_29 = 258, CL_38  = 264, CL_50 = 262,
        CL_54 = 261, CL_62 = 259, CL_102 = 260
    };

    enum class DieCut : int {
        DC_17x54  = 269, DC_17x87  = 270, DC_23x23   = 370, DC_29x90 = 271,
        DC_38x90  = 272, DC_39x48  = 367, DC_52x29   = 374, DC_62x29 = 274,
        DC_62x100 = 275, DC_102x51 = 365, DC_102x152 = 366
    };

    const std::map<ContinuousLength, LabelDimensions> continuous_length_dimensions {
            {ContinuousLength::CL_12,  { 0,  12, 0, 106  }},
            {ContinuousLength::CL_29,  { 0,  29, 0, 306  }},
            {ContinuousLength::CL_38,  { 0,  38, 0, 413  }},
            {ContinuousLength::CL_50,  { 0,  50, 0, 554  }},
            {ContinuousLength::CL_54,  { 0,  54, 0, 590  }},
            {ContinuousLength::CL_62,  { 0,  62, 0, 696  }},
            {ContinuousLength::CL_102, { 0, 102, 0, 1164 }}
    };

    const std::map<DieCut, LabelDimensions> die_cut_dimensions {
            {LabelSubtypes::DieCut::DC_17x54,   { 54,   17,  566, 165  }},
            {LabelSubtypes::DieCut::DC_17x87,   { 87,   17,  956, 165  }},
            {LabelSubtypes::DieCut::DC_23x23,   { 23,   23,  202, 236  }},
            {LabelSubtypes::DieCut::DC_29x90,   { 90,   29,  991, 306  }},
            {LabelSubtypes::DieCut::DC_38x90,   { 90,   38,  991, 413  }},
            {LabelSubtypes::DieCut::DC_39x48,   { 48,   39,  495, 425  }},
            {LabelSubtypes::DieCut::DC_52x29,   { 29,   52,  271, 578  }},
            {LabelSubtypes::DieCut::DC_62x29,   { 29,   62,  271, 696  }},
            {LabelSubtypes::DieCut::DC_62x100,  { 100,  62, 1109, 696  }},
            {LabelSubtypes::DieCut::DC_102x51,  { 51,  102,  526, 1164 }},
            {LabelSubtypes::DieCut::DC_102x152, { 152, 102, 1660, 1164 }}
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
    const std::string discard_date;

    std::vector<uint8_t> prepare_for_printing(cairo_surface_t *surface) const noexcept;
    inline static bool thresh(const unsigned char *pix, int threshold = 190) noexcept;

    static Label from_yaml_node(const YAML::Node& node, ProductUsage usage);

public:
    Label(std::string product, ProductUsage usage, std::string start, std::optional<std::string> ready, std::string discard) noexcept;
    Label(std::string product, ProductUsage usage, std::string start, std::string discard) noexcept;

    static void set_die_cut_label_type(LabelSubtypes::DieCut _label_type, bool high_quality = false) noexcept;
    static void set_continuous_length_label_type(LabelSubtypes::ContinuousLength _label_type, int width_mm, bool high_quality = false) noexcept;

    [[nodiscard]] std::vector<uint8_t> get_printing_data() const noexcept;

    [[nodiscard]] static std::vector<Label> load_label_definitions(const std::string& def_file);

    friend class LabelCreator;
    friend class PrinterJobData;
};


#endif //LABEL_PRINTER_DRIVER_LABEL_H
