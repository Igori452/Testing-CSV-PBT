#include "include/user_wrapper.hpp"
#include "include/csv_tester.hpp"

int main() {
    //"rapidcsv", "fastcsv", "pedrovicente"
    auto parser = ParserManager::getParser("pedrovicente");

    VerificationCSV verificationCSV(parser.get());
    verificationCSV.verification();

    CheckMetricsCSV checkMetricsCSV(parser.get());
    checkMetricsCSV.checkRobustness();
    checkMetricsCSV.checkRecovery();
    checkMetricsCSV.checkPerformance();
    checkMetricsCSV.checkDataPreservation();
    checkMetricsCSV.checkRoundtrip();
    checkMetricsCSV.checkStructuralIntegrity();

    return 0;
}
