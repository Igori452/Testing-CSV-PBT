#include "../include/user_wrapper.h"

#include "../external/rapidcsv/src/rapidcsv.h"
#include <chrono>

// ===== ТУТ ПОЛЬЗОВАТЕЛЬ ПИШЕТ СВОИ РЕАЛИЗАЦИИ =====

// Пример класса адаптера для тестирования парсера RapidCSV
class RapidCSV : public CSVParser {
    public:
        ParseResult parse (const std::string& csv_data) override {
            ParseResult result;
            auto start = std::chrono::steady_clock::now();
            
            try {
                std::stringstream ss(csv_data);
                rapidcsv::Document doc(ss, rapidcsv::LabelParams(-1, -1)); // без заголовков
                
                std::vector<std::vector<std::string>> rows;
                
                // Просто читаем все как строки
                for (size_t row = 0; row < doc.GetRowCount(); ++row) {
                    std::vector<std::string> rowData;
                    for (size_t col = 0; col < doc.GetColumnCount(); ++col) {
                        rowData.push_back(doc.GetCell<std::string>(col, row));
                    }
                    rows.push_back(rowData);
                }
                
                result.data = rows;
                result.error = "";
                
            } catch (const std::exception& e) {
                result.error = e.what();
            }
            
            auto end = std::chrono::steady_clock::now();
            result.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
            
            return result;
        }

        std::string name() const override {
            return "RapidCsv";    
        }
};


// ===== ТУТ ПОЛЬЗОВАТЕЛЬ РЕГИСТРИРУЕТ СВОИ РЕАЛИЗАЦИИ =====

static auto _ = (ParserManager::registerParser("rapidcsv", []() {
    return std::make_unique<RapidCSV>();
}), 0);