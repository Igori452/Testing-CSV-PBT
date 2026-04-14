#include "../include/csv_tester.hpp"
#include "../include/rapidcheck_generator.hpp"
#include <iomanip>
#include <chrono>

#include <fstream>
#include <iostream>
#include <unordered_set>

static std::string serializeTable(const std::vector<std::vector<std::string>>& table) {
    std::string result;
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[i].size(); j++) {
            // Базовое экранирование для надежности
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
    std::cout << "  " << (no_crash ? "PASSED" : "FAILED") << "\n\n";

    // Property 2: Idempotence (parsing same string twice gives same result)
    std::cout << "[Property 2] Idempotence\n";
    bool idempotence = rc::check("Idempotence", [this]() {
        const auto csv = *rc::gen::csvString();
        const auto first  = parser->parse(csv);
        const auto second = parser->parse(csv);
        RC_ASSERT(first.data == second.data);
    });
    std::cout << "  " << (idempotence ? "PASSED" : "FAILED") << "\n\n";

    // Property 3: Empty fields handling
    std::cout << "[Property 3] Empty fields\n";
    bool empty = rc::check("Empty fields", [this]() {
        const std::string csv = ",,,,\n";
        const auto result = parser->parse(csv);
        RC_ASSERT(result.data.size() == 1);
        RC_ASSERT(result.data[0].size() == 5);
    });
    std::cout << "  " << (empty ? "PASSED" : "FAILED") << "\n\n";

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
    std::cout << "  " << (recovery ? "PASSED" : "FAILED") << "\n\n";

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
    std::cout << "  " << (boundary ? "PASSED" : "FAILED") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "VERIFICATION COMPLETE\n";
    std::cout << "========================================\n";
}

CheckMetricsCSV::CheckMetricsCSV(CSVParser* p) : parser(p) {};
void CheckMetricsCSV::setParser(CSVParser* p) {
    parser = p;
}

void CheckMetricsCSV::checkRecovery(int tests) {
    int zeroDataLoss = 0;
    
    // Открываем файл для записи всех примеров
    std::string file_name = "recovery_examples_" + parser->name() + ".txt";
    std::ofstream out(file_name);
    out << "=== RECOVERY RATE TEST EXAMPLES ===\n";
    out << "Parser: " << parser->name() << "\n";
    out << "Total tests: " << tests << "\n";
    out << "Recovery condition: parsedRows >= originalRows/2\n";
    out << "====================================\n\n";
    
    int exampleNum = 0;
    
    rc::check("Recovery", [this, &zeroDataLoss, &out, &exampleNum, tests]() {
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        size_t originalRows = table.size();
        
        auto corrupted = *rc::gen::corruptedCSV(csv);
        if (corrupted.empty()) corrupted = csv;
        
        try {
            ParseResult parsed = parser->parse(corrupted);
            size_t parsedRows = parsed.data.size();
            
            // Записываем каждый тест в файл
            exampleNum++;
            out << "[Test " << exampleNum << "]\n";
            out << "Original CSV (clean): " << csv << "\n";
            out << "Original rows: " << originalRows << "\n";
            out << "Corrupted CSV: " << corrupted << "\n";
            out << "Parser extracted:\n";
            
            if (parsed.data.empty()) {
                out << "  (empty result)\n";
            } else {
                for (size_t i = 0; i < parsed.data.size(); i++) {
                    out << "  Row " << i << ": ";
                    if (parsed.data[i].empty()) {
                        out << "(empty row)";
                    } else {
                        for (size_t j = 0; j < parsed.data[i].size(); j++) {
                            out << "[" << parsed.data[i][j] << "]";
                            if (j < parsed.data[i].size() - 1) out << ",";
                        }
                    }
                    out << "\n";
                }
            }
            out << "Parsed rows: " << parsedRows << "\n";
            
            // Проверяем условие восстановления
            bool recovered = (parsedRows > 0 && parsedRows >= originalRows / 2);
            out << "Recovered (parsedRows >= originalRows/2): " 
                << (recovered ? "YES" : "NO") << "\n";
            out << "---\n";
            
            if (recovered) {
                zeroDataLoss++;
            }
        } catch (const std::exception& e) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Original CSV (clean): " << csv << "\n";
            out << "Original rows: " << originalRows << "\n";
            out << "Corrupted CSV: " << corrupted << "\n";
            out << "Error: " << e.what() << "\n";
            out << "Recovered: NO (crash)\n";
            out << "---\n";
        } catch (...) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Original CSV (clean): " << csv << "\n";
            out << "Original rows: " << originalRows << "\n";
            out << "Corrupted CSV: " << corrupted << "\n";
            out << "Error: unknown\n";
            out << "Recovered: NO (crash)\n";
            out << "---\n";
        }
    });
    
    out << "\n=== RESULT ===\n";
    out << "Successful recoveries: " << zeroDataLoss << "/" << tests << "\n";
    out << "Recovery rate: " << std::fixed << std::setprecision(2)
        << (100.0 * zeroDataLoss / tests) << "%\n";
    out.close();
    
    std::cout << "Recovery rate: " << std::fixed << std::setprecision(2)
              << (100.0 * zeroDataLoss / tests) << "%\n";
    std::cout << "Details saved to: " << file_name << "\n";
}

void CheckMetricsCSV::checkDataPreservation(int tests) {
    int fullPreservation = 0;
    
    // Открываем файл для записи всех примеров
    std::string file_name = "datapreservation_examples_" + parser->name() + ".txt";
    std::ofstream out(file_name);
    out << "=== DATA PRESERVATION TEST EXAMPLES ===\n";
    out << "Parser: " << parser->name() << "\n";
    out << "Total tests: " << tests << "\n";
    out << "Data preservation condition: originalCells == parsedCells\n";
    out << "========================================\n\n";
    
    int exampleNum = 0;
    
    rc::check("Data preservation", [this, &fullPreservation, &out, &exampleNum]() {
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        
        // Подсчёт ячеек
        size_t originalCells = 0;
        size_t parsedCells = 0;

        try {
            ParseResult parsed = parser->parse(csv);
            
            for (auto& row : table) originalCells += row.size();
            for (auto& row : parsed.data) parsedCells += row.size();
            
            // Записываем каждый тест в файл
            exampleNum++;
            out << "[Test " << exampleNum << "]\n";
            out << "Input CSV: " << csv << "\n";
            out << "Original table structure:\n";
            for (size_t i = 0; i < table.size(); i++) {
                out << "  Row " << i << ": ";
                for (size_t j = 0; j < table[i].size(); j++) {
                    out << "[" << table[i][j] << "]";
                    if (j < table[i].size() - 1) out << ",";
                }
                out << "\n";
            }
            out << "Original cells count: " << originalCells << "\n";
            
            out << "Parser extracted:\n";
            if (parsed.data.empty()) {
                out << "  (empty result)\n";
            } else {
                for (size_t i = 0; i < parsed.data.size(); i++) {
                    out << "  Row " << i << ": ";
                    if (parsed.data[i].empty()) {
                        out << "(empty row)";
                    } else {
                        for (size_t j = 0; j < parsed.data[i].size(); j++) {
                            out << "[" << parsed.data[i][j] << "]";
                            if (j < parsed.data[i].size() - 1) out << ",";
                        }
                    }
                    out << "\n";
                }
            }
            out << "Parsed cells count: " << parsedCells << "\n";
            
            bool preserved = (originalCells == parsedCells);
            out << "Data preserved (originalCells == parsedCells): " 
                << (preserved ? "YES" : "NO") << "\n";
            
            if (!preserved) {
                out << "Difference: " << (int)originalCells - (int)parsedCells 
                    << " cells lost\n";
            }
            out << "---\n";
            
            if (preserved) {
                fullPreservation++;
            }
        } catch (const std::exception& e) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Input CSV: " << csv << "\n";
            out << "Original cells count: " << originalCells << "\n";
            out << "Error: " << e.what() << "\n";
            out << "Data preserved: NO (crash)\n";
            out << "---\n";
        } catch (...) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Input CSV: " << csv << "\n";
            out << "Original cells count: " << originalCells << "\n";
            out << "Error: unknown\n";
            out << "Data preserved: NO (crash)\n";
            out << "---\n";
        }
    });
    
    out << "\n=== RESULT ===\n";
    out << "Full data preservation: " << fullPreservation << "/" << tests << "\n";
    out << "Data preservation rate: " << std::fixed << std::setprecision(2)
        << (100.0 * fullPreservation / tests) << "%\n";
    out.close();
    
    std::cout << "Data preservation: " << std::fixed << std::setprecision(2)
              << (100.0 * fullPreservation / tests) << "%\n";
    std::cout << "Details saved to: " << file_name << "\n";
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
    
    std::cout << "Average parse time: " << std::fixed << std::setprecision(3)
              << (totalTimeMs / tests) << " ms\n";
}

void CheckMetricsCSV::checkRoundtrip(int tests) {
    int perfectRoundtrip = 0;
    
    // Открываем файл для записи всех примеров
    std::string file_name = "roundtrip_examples_"+ parser->name() +".txt";
    std::ofstream out(file_name);
    out << "=== ROUNDTRIP TEST EXAMPLES ===\n";
    out << "Parser: " << parser->name() << "\n";
    out << "Total tests: " << tests << "\n";
    out << "================================\n\n";
    
    int exampleNum = 0;
    
    rc::check("Roundtrip", [this, &perfectRoundtrip, &out, &exampleNum]() {
        auto table = *rc::gen::csvTable(3, 3);
        auto csv = serializeTable(table);
        
        try {
            ParseResult parsed = parser->parse(csv);
            
            // Записываем каждый тест в файл
            exampleNum++;
            out << "[Test " << exampleNum << "]\n";
            out << "Input CSV: " << csv << "\n";
            out << "Parser extracted:\n";
            
            if (parsed.data.empty()) {
                out << "  (empty result)\n";
            } else {
                for (size_t i = 0; i < parsed.data.size(); i++) {
                    out << "  Row " << i << ": ";
                    if (parsed.data[i].empty()) {
                        out << "(empty row)";
                    } else {
                        for (size_t j = 0; j < parsed.data[i].size(); j++) {
                            out << "[" << parsed.data[i][j] << "]";
                            if (j < parsed.data[i].size() - 1) out << ",";
                        }
                    }
                    out << "\n";
                }
            }
            out << "---\n";
            
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
        } catch (const std::exception& e) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Input CSV: " << csv << "\n";
            out << "Error: " << e.what() << "\n";
            out << "---\n";
        } catch (...) {
            exampleNum++;
            out << "[Test " << exampleNum << "] (CRASH)\n";
            out << "Input CSV: " << csv << "\n";
            out << "Error: unknown\n";
            out << "---\n";
        }
    });
    
    out << "\n=== RESULT ===\n";
    out << "Perfect roundtrips: " << perfectRoundtrip << "/" << tests << "\n";
    out.close();
    
    std::cout << "\nRoundtrip success: " << std::fixed << std::setprecision(2)
              << (100.0 * perfectRoundtrip / tests) << "%\n";
    std::cout << "Details saved to: roundtrip_examples.txt\n";
}