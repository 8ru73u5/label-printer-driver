#include <iostream>

#include "printer/Printer.h"
#include "printer/PrinterStatus.h"
#include "label/Label.h"

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
    Label label("Bułki 5\"", ProductUsage::HEATING, "21.02 08:00", "21.02 14:00", "21.02 16:00");

    PrinterJobData job_data(label);
    job_data.set_quality(false, false);

    std::vector<Label> labels = {label};

    char choice;
    cout << "Print test page? [y/n]: ";
    std::cin >> choice;
    if(choice == 'y') {
        printer.print(labels, job_data);
    }

    return 0;
}
