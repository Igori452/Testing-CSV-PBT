#include "include/user_wrapper.hpp"
#include "include/csv_tester.hpp"

int main() {
    // Достаем данные из seed
    const char* dumpEnv = std::getenv("DUMP_CSV_SAMPLES");
    if (dumpEnv && std::string(dumpEnv) == "1") {
        dumpGeneratedSamples();
        return 0;
    }

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

    return 0;
}
