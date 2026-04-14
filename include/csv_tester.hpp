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
		void checkRecovery(int tests = 300);				// 1. Recovery metric
		void checkDataPreservation(int tests = 300); 		// 2. Data loss metric
		void checkPerformance(int tests = 200);				// 3. Average parse time
		void checkRoundtrip(int tests = 200);				// 4. Roundtrip metric
	};

#endif