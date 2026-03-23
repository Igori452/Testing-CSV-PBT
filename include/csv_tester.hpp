#ifndef CSV_TESTER_H
#define CSV_TESTER_H

#include "user_wrapper.hpp"

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
		void checkRobustness(int tests = 500);		// 1. Robustness metric (успешное парсирование)
		void checkRecovery(int tests = 300);		// 2. Recovery metric (сравнение с оригиналом после порчи)
		void checkConsistency(int tests = 300);		// 3. Structural Consistency (таблица прямоугольная)
		void checkDataLoss(int tests = 300); 		// 4. Data loss metric
		void checkPerformance(int tests = 200);		// 5. Average parse time
};

#endif