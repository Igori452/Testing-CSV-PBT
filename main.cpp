#include "include/user_wrapper.hpp"
#include "include/csv_tester.hpp"

int main() {

    //"rapidcsv", "fastcsv", "pedrovicente"
    auto parser = ParserManager::getParser("pedrovicente");

    VerificationCSV verificationCSV(parser.get());
    verificationCSV.verification();

    CheckMetricsCSV checkMetricsCSV(parser.get());
    checkMetricsCSV.checkRecovery();
    checkMetricsCSV.checkDataPreservation();
    checkMetricsCSV.checkRoundtrip();
    checkMetricsCSV.checkPerformance();

    return 0;
}
