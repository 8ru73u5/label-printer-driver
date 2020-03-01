#ifndef LABEL_PRINTER_DRIVER_LABEL_H
#define LABEL_PRINTER_DRIVER_LABEL_H

#include <string>
#include <map>
#include <vector>

/**
 * Simple struct for holding label dimensions both in millimeters and pixels (points).
 */
struct LabelDimensions {
    uint8_t width_mm;
    uint8_t height_mm;
    uint32_t width_pt;
    uint32_t height_pt;
};

/**
 * Enum which holds all possible label types.
 *
 * `LabelType::UNDEFINED` is just for indication that the label type hasn't been set yet.
 */
enum class LabelType {
    DIE_CUT,
    CONTINUOUS_LENGTH,
    UNDEFINED
};

/**
 * Namespace for label subtypes definition (die-cut and continuous length).
 *
 * For each enum type there is corresponding map which defines `LabelDimensions`
 * for each subtype.
 *
 * @see LabelDimensions
 */
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

/**
 * Base class for label representation.
 *
 * If you want to create another subtype of label (for example QR code label)
 * you have to implement `Label::get_printing_data()` function.
 */
class Label {
protected:
    static LabelType type;
    static LabelDimensions dimensions;

public:
    /**
     * Default constructor that checks if `type` is set (is not `LabelType::UNDEFINED`.
     *
     * @throws std::runtime_exception if `type` is set to `LabelType::UNDEFINED`
     * @see LabelType
     */
    Label();
    virtual ~Label() = default;

    /**
     * Sets `type = LabelType:DIE_CUT`
     *
     * @param label_type Specific `DieCut` label type
     *
     * @throws std::runtime_error if `label_type` is not supported
     * @see LabelSubtypes::DieCut
     */
    static void set_die_cut_label_type(LabelSubtypes::DieCut label_type);

    /**
     * Sets `type = LabelType::CONTINUOUS_LENGTH`
     *
     * @param label_type Specific `ContinuousLength` label type
     * @param width_mm Desired width of the label in millimeters
     *
     * @throws std::runtime_error if `label_type` is not supported
     * @throws std::invalid_argument if label width is not in allowed range
     * @see LabelSubtypes::ContinuousLength
     */
    static void set_continuous_length_label_type(LabelSubtypes::ContinuousLength label_type, int width_mm);

    static LabelType get_type();
    static LabelDimensions get_dimensions();

    /**
     * @return `true` if `type` is not `LabelType::UNDEFINED`
     */
    static bool is_valid();

    /**
     * Construct and return printing data packet.
     *
     * The printing data packet consists of *n* 93 bytes sub-packets where
     * the first 3 bytes is a print data command and the rest is raster data
     * composed of `1` or `0` values for each pixel. *n* is equal to `dimensions.width_pt`.
     * Each sub-packet contains raster data of one column of the label image.
     *
     * @return printing data packet as `std::vector` of bytes
     */
    [[nodiscard]] virtual std::vector<uint8_t> get_printing_data() const = 0;
};


#endif //LABEL_PRINTER_DRIVER_LABEL_H
