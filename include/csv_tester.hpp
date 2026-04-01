#ifndef CSV_TESTER_H
#define CSV_TESTER_H

#include "user_wrapper.hpp"

void dumpGeneratedSamples(const std::string& filename = "generated_csv_samples.txt", int count = 500);

class VerificationCSV {
	private:
		CSVParser* parser;
	public:
		VerificationCSV() = delete;
		VerificationCSV(CSVParser* p);
		void setParser(CSVParser* p);
		void verification();
};

class CheckMetricsCSV {
	private:
		CSVParser* parser;
	public:
		CheckMetricsCSV() = delete;
		CheckMetricsCSV(CSVParser* p);
		void setParser(CSVParser* p);
		void checkRobustness(int tests = 500);				// 1. Robustness metric
		void checkRecovery(int tests = 300);				// 2. Recovery metric
		void checkStructuralIntegrity(int tests = 300);		// 3. Structural Consistency metric
		void checkDataPreservation(int tests = 300); 		// 4. Data loss metric
		void checkPerformance(int tests = 200);				// 5. Average parse time
		void checkRoundtrip(int tests = 200);				// 6. Roundtrip metric
	};

#endif