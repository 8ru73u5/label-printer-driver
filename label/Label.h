#ifndef LABEL_PRINTER_DRIVER_LABEL_H
#define LABEL_PRINTER_DRIVER_LABEL_H

#include <string>
#include <map>
#include <vector>

struct LabelDimensions {
    uint8_t width_mm;
    uint8_t height_mm;
    uint32_t width_pt;
    uint32_t height_pt;
};

enum class LabelType {
    DIE_CUT,
    CONTINUOUS_LENGTH,
    UNDEFINED
};

namespace LabelSubtypes {
    enum class ContinuousLength : int {
        CL_12 = 257, CL_29 = 258, CL_38 = 264, CL_50 = 262,
        CL_54 = 261, CL_62 = 259
    };

    enum class DieCut : int {
        DC_17x54  = 269, DC_17x87 = 270, DC_23x23 = 370, DC_29x90 = 271,
        DC_38x90  = 272, DC_39x48 = 367, DC_52x29 = 374, DC_62x29 = 274,
        DC_62x100 = 275
    };

    const std::map<ContinuousLength, LabelDimensions> __continuous_length_dimensions {
            {ContinuousLength::CL_12,  { 0,  12, 0, 106  }},
            {ContinuousLength::CL_29,  { 0,  29, 0, 306  }},
            {ContinuousLength::CL_38,  { 0,  38, 0, 413  }},
            {ContinuousLength::CL_50,  { 0,  50, 0, 554  }},
            {ContinuousLength::CL_54,  { 0,  54, 0, 590  }},
            {ContinuousLength::CL_62,  { 0,  62, 0, 696  }}
    };

    const std::map<DieCut, LabelDimensions> __die_cut_dimensions {
            {LabelSubtypes::DieCut::DC_17x54,   { 54,   17,  566, 165  }},
            {LabelSubtypes::DieCut::DC_17x87,   { 87,   17,  956, 165  }},
            {LabelSubtypes::DieCut::DC_23x23,   { 23,   23,  202, 236  }},
            {LabelSubtypes::DieCut::DC_29x90,   { 90,   29,  991, 306  }},
            {LabelSubtypes::DieCut::DC_38x90,   { 90,   38,  991, 413  }},
            {LabelSubtypes::DieCut::DC_39x48,   { 48,   39,  495, 425  }},
            {LabelSubtypes::DieCut::DC_52x29,   { 29,   52,  271, 578  }},
            {LabelSubtypes::DieCut::DC_62x29,   { 29,   62,  271, 696  }},
            {LabelSubtypes::DieCut::DC_62x100,  { 100,  62, 1109, 696  }}
    };
}

class Label {
protected:
    static LabelType type;
    static LabelDimensions dimensions;

public:
    Label();
    virtual ~Label() = default;

    static void set_die_cut_label_type(LabelSubtypes::DieCut label_type, bool high_quality = false);
    static void set_continuous_length_label_type(LabelSubtypes::ContinuousLength label_type, int width_mm, bool high_quality = false);

    static LabelType get_type();
    static LabelDimensions get_dimensions();

    static bool is_valid();

    [[nodiscard]] virtual std::vector<uint8_t> get_printing_data() const = 0;
};


#endif //LABEL_PRINTER_DRIVER_LABEL_H
