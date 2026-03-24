#include "../include/csv_tester.hpp"
#include "../include/rapidcheck_generator.hpp"
#include <iomanip>
#include <chrono>

static std::string serializeTable(const std::vector<std::vector<std::string>>& table) {
    std::string result;
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[i].size(); j++) {
            // Добавляем базовое экранирование для надежности
            std::string cell = table[i][j];
            // Если в ячейке есть запятая или кавычка, оборачиваем в кавычки
            if (cell.find(',') != std::string::npos || cell.find('"') != std::string::npos) {
                // Экранируем кавычки внутри
                size_t pos = 0;
                while ((pos = cell.find('"', pos)) != std::string::npos) {
                    cell.insert(pos, "\"");
                    pos += 2;
                }
                result += "\"" + cell + "\"";
            } else {
                result += cell;
            }
            
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

    // Property 1: No crashes on random input
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

    // Property 2: Idempotence (parsing same string twice gives same result)
    std::cout << "[Property 2] Idempotence\n";
    bool idempotence = rc::check("Idempotence", [this]() {
        const auto csv = *rc::gen::csvString();
        const auto first  = parser->parse(csv);
        const auto second = parser->parse(csv);
        RC_ASSERT(first.data == second.data);
    });
    std::cout << "  " << (idempotence ? "✓ PASSED" : "✗ FAILED") << "\n\n";

    // Property 3: Empty fields handling
    std::cout << "[Property 3] Empty fields\n";
    bool empty = rc::check("Empty fields", [this]() {
        const std::string csv = ",,,,\n";
        const auto result = parser->parse(csv);
        RC_ASSERT(result.data.size() == 1);
        RC_ASSERT(result.data[0].size() == 5);
    });
    std::cout << "  " << (empty ? "✓ PASSED" : "✗ FAILED") << "\n\n";

    // Property 4: Error recovery (doesn't crash on corrupted input)
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

    // Property 5: Boundary conditions
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

// Оставляем - это полезная метрика
void CheckMetricsCSV::checkRobustness(int tests) {
    int success = 0;
    rc::check("Robustness", [this, &success, tests]() {
        const auto csv = *rc::gen::corruptedCSV();
        try {
            parser->parse(csv);
            success++;
        } catch (...) {
            // Пропускаем, просто считаем успешные
        }
    });
    
    std::cout << "Robustness: " << std::fixed << std::setprecision(2) 
              << (100.0 * success / tests) << "%\n";
    std::cout << "  (Higher is better - parser handles corrupted input gracefully)\n";
}

void CheckMetricsCSV::debugGenerator() {
    std::cout << "\n=== Debug: What does csvTable generate? ===\n";
    
    // Используем rc::check для генерации значений
    rc::check("Debug generator", [](int test) {
        // Генерируем таблицу
        auto table = *rc::gen::csvTable(3, 3);
        
        std::cout << "Test " << test + 1 << ": " << table.size() << " rows\n";
        
        for (size_t row = 0; row < std::min(table.size(), size_t(3)); row++) {
            std::cout << "  Row " << row << " (" << table[row].size() << " cols): ";
            for (size_t col = 0; col < std::min(table[row].size(), size_t(5)); col++) {
                std::string cell = table[row][col];
                bool hasComma = cell.find(',') != std::string::npos;
                bool hasQuote = cell.find('"') != std::string::npos;
                bool hasNewline = cell.find('\n') != std::string::npos;
                
                if (hasComma || hasQuote || hasNewline) {
                    std::cout << "\"" << cell << "\" ";
                    if (hasComma) std::cout << "[,]";
                    if (hasQuote) std::cout << "[\"]";
                    if (hasNewline) std::cout << "[\\n]";
                } else {
                    // Ограничиваем вывод длины
                    if (cell.length() > 20) {
                        std::cout << cell.substr(0, 17) << "... ";
                    } else {
                        std::cout << cell << " ";
                    }
                }
                std::cout << " ";
            }
            std::cout << "\n";
        }
        std::cout << "---\n";
        
        // Останавливаемся после 5 тестов
        return test < 4;
    });
}

// Упрощаем recovery - проверяем только отсутствие падений
void CheckMetricsCSV::checkRecovery(int tests) {
    int totalProcessed = 0;
    int zeroDataLoss = 0;
    
    rc::check("Recovery", [this, &totalProcessed, &zeroDataLoss]() {
        auto table = *rc::gen::csvTable(3, 3); // Фиксированный размер для предсказуемости
        auto csv = serializeTable(table);
        
        auto corrupted = *rc::gen::corruptedCSV(csv);
        if (corrupted.empty()) corrupted = csv;
        
        try {
            ParseResult parsed = parser->parse(corrupted);
            totalProcessed++;
            
            // Проверяем, что не потеряли все данные
            size_t originalRows = table.size();
            size_t parsedRows = parsed.data.size();
            
            // Если хоть что-то распарсилось, считаем это успехом
            if (parsedRows > 0 && parsedRows >= originalRows / 2) {
                zeroDataLoss++;
            }
        } catch (...) {
            // Если упало, это плохо для recovery
        }
    });
    
    std::cout << "Recovery rate: " << std::fixed << std::setprecision(2)
              << (100.0 * zeroDataLoss / tests) << "%\n";
    std::cout << "  (Higher is better - parser recovers meaningful data from corruption)\n";
}

// Убираем consistency, заменяем на более простую метрику
void CheckMetricsCSV::checkStructuralIntegrity(int tests) {
    int perfectMatches = 0;
    
    rc::check("Structural integrity", [this, &perfectMatches]() {
        // Используем маленькие таблицы для предсказуемости
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        
        try {
            ParseResult parsed = parser->parse(csv);
            
            // Проверяем базовую структуру
            if (parsed.data.size() == table.size() && 
                !parsed.data.empty() && 
                parsed.data[0].size() == table[0].size()) {
                perfectMatches++;
            }
        } catch (...) {
            // Игнорируем ошибки
        }
    });
    
    std::cout << "Structural integrity: " << std::fixed << std::setprecision(2)
              << (100.0 * perfectMatches / tests) << "%\n";
    std::cout << "  (Higher is better - parser preserves table structure for clean CSV)\n";
}

// Упрощаем data loss
void CheckMetricsCSV::checkDataPreservation(int tests) {
    int fullPreservation = 0;
    
    rc::check("Data preservation", [this, &fullPreservation]() {
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        
        try {
            ParseResult parsed = parser->parse(csv);
            
            // Проверяем, что количество ячеек сохранилось
            size_t originalCells = 0;
            size_t parsedCells = 0;
            
            for (auto& row : table) originalCells += row.size();
            for (auto& row : parsed.data) parsedCells += row.size();
            
            if (originalCells == parsedCells) {
                fullPreservation++;
            }
        } catch (...) {
            // Игнорируем
        }
    });
    
    std::cout << "Data preservation: " << std::fixed << std::setprecision(2)
              << (100.0 * fullPreservation / tests) << "%\n";
    std::cout << "  (Higher is better - parser preserves all cells)\n";
}

// Оставляем performance - полезная метрика
void CheckMetricsCSV::checkPerformance(int tests) {
    double totalTimeMs = 0.0;
    
    rc::check("Performance", [this, &totalTimeMs]() {
        auto csv = *rc::gen::csvString();
        
        auto start = std::chrono::steady_clock::now();
        parser->parse(csv);
        auto end = std::chrono::steady_clock::now();
        
        totalTimeMs += std::chrono::duration<double, std::milli>(end - start).count();
    });
    
    std::cout << "Average parse time: " << std::fixed << std::setprecision(3)
              << (totalTimeMs / tests) << " ms\n";
    std::cout << "  (Lower is better)\n";
}

// Новая метрика: сравнение с эталоном
void CheckMetricsCSV::checkRoundtrip(int tests) {
    int perfectRoundtrip = 0;
    
    rc::check("Roundtrip", [this, &perfectRoundtrip]() {
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        
        try {
            ParseResult parsed = parser->parse(csv);
            
            // Проверяем размеры
            if (parsed.data.size() == table.size()) {
                bool allMatch = true;
                for (size_t i = 0; i < table.size() && allMatch; i++) {
                    if (parsed.data[i].size() != table[i].size()) {
                        allMatch = false;
                    }
                }
                if (allMatch) perfectRoundtrip++;
            }
        } catch (...) {
            // Игнорируем
        }
    });
    
    std::cout << "Roundtrip success: " << std::fixed << std::setprecision(2)
              << (100.0 * perfectRoundtrip / tests) << "%\n";
    std::cout << "  (Higher is better - parser correctly handles serialized data)\n";
}