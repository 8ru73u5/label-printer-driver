#ifndef LABEL_PRINTER_DRIVER_PRODUCTLABEL_H
#define LABEL_PRINTER_DRIVER_PRODUCTLABEL_H

#include <ctime>
#include <cairo/cairo.h>
#include <yaml-cpp/yaml.h>

#include "Label.h"

enum class ProductUsage {
    BOARD,
    PREP,
    STORAGE
};

class ProductLabel : public Label {
private:
    std::string name;
    ProductUsage usage;
    std::optional<std::time_t> start_date;
    std::optional<std::string> ready_date;
    std::string discard_date;

    [[nodiscard]] std::vector<uint8_t> prepare_for_printing(cairo_surface_t *surface) const;
    [[nodiscard]] static inline bool thresh(const unsigned char *pix, int threshold = 190) noexcept;

public:
    ProductLabel(std::string name, ProductUsage usage, std::optional<std::time_t> start,
            std::optional<std::string> ready, std::string discard) noexcept;
    ProductLabel(const YAML::Node& node, ProductUsage usage);
    ~ProductLabel() override = default;
    [[nodiscard]] static std::vector<Label*> load_label_definitions(const std::string& def_file);

    inline void set_start_date(std::optional<std::time_t> start) noexcept;

    [[nodiscard]] std::vector<uint8_t> get_printing_data() const override;

    friend class ProductLabelCreator;
};


#endif //LABEL_PRINTER_DRIVER_PRODUCTLABEL_H
