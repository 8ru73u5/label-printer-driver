#include <iostream>
#include <algorithm>

#include "printer/Printer.h"
#include "printer/PrinterStatus.h"
#include "label/Label.h"
#include "label/LabelCreator.h"

using std::cout, std::endl;

int main() {
    Printer printer = Printer();
    printer.scan_for_printer();
    cout << endl;

    printer.clear_jobs();
    printer.init();
    PrinterStatus status(printer.send_request_status());
    status.display();

    Label::set_die_cut_label_type(LabelSubtypes::DieCut::DC_29x90);
    Label *label = new ProductLabel("Ketchup", ProductUsage::BOARD, std::nullopt, "2h", "4h");

    LabelCreator::load_config("../label/label_conf.yml");
    auto *png = dynamic_cast<ProductLabel*>(label);
    LabelCreator::export_to_png(*png, "out.png");

    auto label_defs = ProductLabel::load_label_definitions("../label/label_def_example.yml");

    PrinterJobData job_data(*label);
    job_data.set_quality(false);

    std::vector<Label*> labels = {label};

    char choice;
    cout << "Print test page? [y/n]: ";
    std::cin >> choice;
    if(choice == 'y') {
        printer.print(labels, job_data);
    }

    std::for_each(label_defs.begin(), label_defs.end(), [](Label *i) { delete i; });

    return 0;
}
