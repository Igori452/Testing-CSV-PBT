#include "include/user_wrapper.hpp"
#include "include/csv_tester.hpp"
#include <iostream>

int main() {
    //"rapidcsv", "fastcsv", "pedrovicente"
    auto parser = ParserManager::getParser("rapidcsv");

    VerificationCSV verificationCSV(parser.get());
    verificationCSV.verification();

    CheckMetricsCSV checkMetricsCSV(parser.get());
    checkMetricsCSV.checkRobustness();
    checkMetricsCSV.checkRecovery();
    checkMetricsCSV.checkPerformance();
    checkMetricsCSV.checkDataPreservation();
    checkMetricsCSV.checkRoundtrip();
    checkMetricsCSV.checkStructuralIntegrity();

    checkMetricsCSV.debugGenerator();

    return 0;
}
