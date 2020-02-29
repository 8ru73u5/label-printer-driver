#include <iostream>
#include <algorithm>

#include "printer/Printer.h"
#include "printer/PrinterStatus.h"
#include "label/Label.h"
#include "label/ProductLabelCreator.h"

using std::cout, std::endl;

int main() {
    Printer printer = Printer();
    printer.scan_for_printer();
    cout << endl;

    printer.clear_jobs();
    printer.init();
    PrinterStatus status(printer.send_request_status());
    status.display();

    Label::set_continuous_length_label_type(LabelSubtypes::ContinuousLength::CL_29, 40);
    Label *label = new ProductLabel("Ketchup", ProductUsage::BOARD, {}, "2h", "4h");

    ProductLabelCreator::load_config("../label/label_conf.yml");

    // Save printed page
    auto *png = dynamic_cast<ProductLabel*>(label);
    ProductLabelCreator::export_to_png(*png, "out.png");

    PrinterJobData job_data {};
    job_data.set_quality(true);
    job_data.set_cut_at_end(true);

    std::vector<Label*> labels = {label};

    char choice;
    cout << "Print test page? [y/n]: ";
    std::cin >> choice;
    if(choice == 'y') {
        printer.print(labels, job_data);
    }

    return 0;
}
