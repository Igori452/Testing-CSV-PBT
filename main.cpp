#include "include/user_wrapper.hpp"
#include "include/csv_tester.hpp"
#include <iostream>

int main() {
    //"rapidcsv", "fastcsv"
    auto parser = ParserManager::getParser("fastcsv");

    VerificationCSV verificationCSV(parser.get());
    verificationCSV.verification();

    CheckMetricsCSV checkMetricsCSV(parser.get());
    checkMetricsCSV.checkRobustness();
    checkMetricsCSV.checkRecovery();
    checkMetricsCSV.checkConsistency();
    checkMetricsCSV.checkDataLoss();
    checkMetricsCSV.checkPerformance();

    return 0;
}