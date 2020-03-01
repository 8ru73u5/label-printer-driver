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

/**
 * Label subclass that represents a product label which has information
 * about:
 * - product name and its usage
 * - start date (when the product is removed from the fridge or opened etc.)
 * - ready date (when the product can be used because it needs for example time to defrost)
 * - discard date (when the product can no longer be used because it is spoiled or something)
 *
 * @see ProductLabelCreator
 */
class ProductLabel : public Label {
private:
    std::string name;
    ProductUsage usage;
    std::optional<std::time_t> start_date;
    std::optional<std::string> ready_date;
    std::string discard_date;

    /**
     * Takes `cairo_surface_t` and converts it to vector of bytes which
     * represents printing data.
     *
     * @param surface Image in RGB24 format that represents a product label
     * @return printing data
     *
     * @throws std::invalid_argument if image is not in RGB24 format
     *
     * @see get_printing_data()
     */
    [[nodiscard]] std::vector<uint8_t> prepare_for_printing(cairo_surface_t *surface) const;

    /**
     * Little helper function for converting single RGB24 pixel into a boolean value.
     *
     * First it converts the pixel to grayscale and then uses `threshold` to determine
     * if treat it as a black or white.
     *
     * @param pix A pointer to RGB24 pixel
     * @param threshold Black/white threshold
     * @return Pixel converted to a boolean value
     */
    [[nodiscard]] static inline bool thresh(const unsigned char *pix, int threshold = 190) noexcept;

public:
    ProductLabel(std::string name, ProductUsage usage, std::optional<std::time_t> start,
            std::optional<std::string> ready, std::string discard) noexcept;

    /**
     * Constructs `ProductLabel` from a YAML node.
     *
     * The node must contain following keys:
     * - *name*
     * - corresponding usage key (for example *prep* if `usage == ProductUsage::PREP`)
     * - *discard*
     *
     * @param node YAML node which contains required keys to construct a `ProductLabel`
     * @param usage Label usage
     *
     * @throws std::invalid_argument if `usage` is not supported
     * @throws std::runtime_error if the `node` doesn't have required keywords
     */
    ProductLabel(const YAML::Node& node, ProductUsage usage);

    ~ProductLabel() override = default;

    /**
     * Loads label definitions from given config file and stores them
     * in a vector of shared pointers to `Label`.
     *
     * This function can be quite expensive because it constructs every
     * label on the heap (and uses `std::shared_ptr`). It should be invoked
     * only when the program starts or the contents of config file changes.
     *
     * @param def_file Config file (in YAML format) which stores the definitions
     * @return A vector of pointers to loaded labels
     *
     * @throws std::runtime_error if the file doesn't have a *products* key or the *products* key
     * is not a sequence
     */
    [[nodiscard]] static std::vector<std::shared_ptr<Label>> load_label_definitions(const std::string& def_file);

    void set_start_date(std::optional<std::time_t> start) noexcept;

    /**
     * Uses `ProductLabelCreator::create_label_surface(ProductLabel&)` to
     * obtain printing data for the label.
     *
     * @return Printing data constructed from the label
     *
     * @see ProductLabelCreator::create_label_surface(ProductLabel&), prepare_for_printing(cairo_surface_t*),
     * Label::get_printing_data()
     */
    [[nodiscard]] std::vector<uint8_t> get_printing_data() const override;

    friend class ProductLabelCreator;
};


#endif //LABEL_PRINTER_DRIVER_PRODUCTLABEL_H
