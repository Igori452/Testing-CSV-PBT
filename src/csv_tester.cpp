#include "../include/csv_tester.hpp"
#include "../include/rapidcheck_generator.hpp"

static std::string serializeTable(const std::vector<std::vector<std::string>>& table) {
    std::string result;
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[i].size(); j++) {
            result += table[i][j];
            if (j < table[i].size() - 1) result += ",";
        }
        if (i < table.size() - 1) result += "\n";
    }
    return result;
}

VerificationCSV::VerificationCSV(CSVParser* p) : parser(p) {};
void VerificationCSV::setParser(CSVParser* p) {
	parser = p;
}

void VerificationCSV::verification() {
    std::cout << "\n========================================\n";
    std::cout << "PROPERTY-BASED VERIFICATION TESTS\n";
    std::cout << "Parser: " << parser->name() << "\n";
    std::cout << "========================================\n\n";

    // Property 1
    std::cout << "[Property 1] No crashes on random input\n";
    bool no_crash = rc::check("No crashes", [this]() {
        const auto csv = *rc::gen::csvString();

        RC_ASSERT([&]() {
			try {
				parser->parse(csv);
				return true;
			} catch (...) {
				return false;
			}
		}());
    });
    std::cout << "  " << (no_crash ? "✓ PASSED" : "✗ FAILED") << "\n\n";


    // Property 2
    std::cout << "[Property 2] Idempotence\n";
    bool idempotence = rc::check("Idempotence", [this]() {
        const auto csv = *rc::gen::csvString();

        const auto first  = parser->parse(csv);
        const auto second = parser->parse(csv);

        RC_ASSERT(first.data == second.data);
    });
    std::cout << "  " << (idempotence ? "✓ PASSED" : "✗ FAILED") << "\n\n";


    // Property 3
    std::cout << "[Property 3] Empty fields\n";
    bool empty = rc::check("Empty fields", [this]() {
        const std::string csv = ",,,,\n";

        const auto result = parser->parse(csv);

        RC_ASSERT(result.data.size() == 1);
        RC_ASSERT(result.data[0].size() == 5);
    });
    std::cout << "  " << (empty ? "✓ PASSED" : "✗ FAILED") << "\n\n";


    // Property 4
    std::cout << "[Property 4] Error recovery\n";
    bool recovery = rc::check("Error recovery", [this]() {
        const auto corrupted = *rc::gen::corruptedCSV();

        RC_ASSERT([&]() {
			try {
				parser->parse(corrupted);
				return true;
			} catch (...) {
				return false;
			}
		}());
    });
    std::cout << "  " << (recovery ? "✓ PASSED" : "✗ FAILED") << "\n\n";


    // Property 5
    std::cout << "[Property 5] Boundary conditions\n";
    bool boundary = rc::check("Boundary conditions", [this]() {
        const auto csv = *rc::gen::boundaryCSV();

        RC_ASSERT([&]() {
			try {
				parser->parse(csv);
				return true;
			} catch (...) {
				return false;
			}
		}());
    });
    std::cout << "  " << (boundary ? "✓ PASSED" : "✗ FAILED") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "VERIFICATION COMPLETE\n";
    std::cout << "========================================\n";
}


CheckMetricsCSV::CheckMetricsCSV(CSVParser* p) : parser(p) {};
void CheckMetricsCSV::setParser(CSVParser* p) {
    parser = p;
}

void CheckMetricsCSV::checkRobustness(int tests) {
    int success = 0;
    rc::check("Robustness", [this, &success, tests]() {
        const auto csv = *rc::gen::corruptedCSV();
        try {
            parser->parse(csv);
            success++;
        } catch (...) {
            RC_ASSERT(true); // просто пропускаем
        }
    });
    
    std::cout << "Robustness: " << (100.0 * success / tests) << "%\n";
}

void CheckMetricsCSV::checkRecovery(int tests) {
    double totalScore = 0.0;

    rc::check("Recovery score", [this, &totalScore]() {
        auto table = *rc::gen::csvTable();
        auto csv = serializeTable(table);

        // портим CSV
        auto corrupted = *rc::gen::corruptedCSV();
        if (corrupted.empty()) corrupted = csv;

        ParseResult parsed = parser->parse(corrupted);

        // Считаем совпадающие ячейки
        size_t matched = 0;
        size_t total = 0;
        for (size_t i = 0; i < std::min(table.size(), parsed.data.size()); i++) {
            for (size_t j = 0; j < std::min(table[i].size(), parsed.data[i].size()); j++) {
                if (table[i][j] == parsed.data[i][j]) matched++;
                total++;
            }
        }
        if (total > 0) totalScore += (double)matched / total;
    });
    std::cout << "Average Recovery Score: " << (totalScore / tests * 100.0) << "%\n";
}

void CheckMetricsCSV::checkConsistency(int tests) {
    int consistentCount = 0;

    rc::check("Structural consistency", [this, &consistentCount]() {
        auto csv = *rc::gen::csvString();
        ParseResult parsed = parser->parse(csv);

        bool consistent = true;
        if (!parsed.data.empty()) {
            size_t cols = parsed.data[0].size();
            for (auto& row : parsed.data)
                if (row.size() != cols) consistent = false;
        }
        if (consistent) consistentCount++;
    });

    std::cout << "Structural consistency: " << (100.0 * consistentCount / tests) << "%\n";
}

void CheckMetricsCSV::checkDataLoss(int tests) {
    double totalRatio = 0.0;

    rc::check("Data loss", [this, &totalRatio]() {
        auto table = *rc::gen::csvTable();
        auto csv = serializeTable(table);

        ParseResult parsed = parser->parse(csv);

        size_t originalCells = 0;
        size_t parsedCells = 0;

        for (auto& row : table) originalCells += row.size();
        for (auto& row : parsed.data) parsedCells += row.size();

        if (originalCells > 0)
            totalRatio += (double)parsedCells / originalCells;
    });

    std::cout << "Data loss ratio: " << (totalRatio / tests) << "\n";
}

void CheckMetricsCSV::checkPerformance(int tests) {
    double totalTimeMs = 0.0;

    rc::check("Performance", [this, &totalTimeMs]() {
        auto csv = *rc::gen::csvString();

        auto start = std::chrono::steady_clock::now();
        parser->parse(csv);
        auto end = std::chrono::steady_clock::now();

        totalTimeMs += std::chrono::duration<double, std::milli>(end - start).count();
    });
    
    std::cout << "Average parse time: " << (totalTimeMs / tests) << " ms\n";
}
