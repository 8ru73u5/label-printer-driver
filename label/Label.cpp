#include <cmath>

#include "Label.h"

LabelType Label::type = LabelType::UNDEFINED;
LabelDimensions Label::dimensions = {};

Label::Label() {
    if(!is_valid())
        throw std::runtime_error("Label type not set! Use provided static function to set it");
}

void Label::set_die_cut_label_type(LabelSubtypes::DieCut _label_type) {
    const auto& lab_it = LabelSubtypes::__die_cut_dimensions.find(_label_type);
    if(lab_it == LabelSubtypes::__die_cut_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    Label::type = LabelType::DIE_CUT;
    Label::dimensions = lab_it->second;
}

void Label::set_continuous_length_label_type(LabelSubtypes::ContinuousLength _label_type, const int width_mm) {
    const auto& lab_it = LabelSubtypes::__continuous_length_dimensions.find(_label_type);
    if(lab_it == LabelSubtypes::__continuous_length_dimensions.end())
        throw std::runtime_error("This label type is not supported");

    if(Label::dimensions.width_pt < 13 || Label::dimensions.width_pt > 1000)
        throw std::invalid_argument("Label width must be in range <13mm, 1000mm>");

    Label::type = LabelType::CONTINUOUS_LENGTH;
    Label::dimensions.width_mm = width_mm;
    Label::dimensions.height_mm = lab_it->second.height_mm;
    Label::dimensions.width_pt = static_cast<uint32_t>(round(width_mm * 0.03937 * 300));
    Label::dimensions.height_pt = lab_it->second.height_pt;
}

LabelType Label::get_type() {
    return type;
}

LabelDimensions Label::get_dimensions() {
    return dimensions;
}

bool Label::is_valid() {
    return type != LabelType::UNDEFINED;
}
