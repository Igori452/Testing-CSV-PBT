#ifndef CSV_TESTER_H
#define CSV_TESTER_H

#include <vector>
#include <string>

class CSVParser;

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
		void checkMetrick_1();
		void checkMetrick_2();
		void checkMetrick_3();
		// ...
};

#endif