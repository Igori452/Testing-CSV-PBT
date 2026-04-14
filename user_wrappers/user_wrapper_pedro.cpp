#include "../include/user_wrapper.hpp"
#include "../external/csv-parser/csv.hh"


// Пример класса адаптера для тестирования парсера csv-parser
class PedroVicenteCSVParser : public CSVParser {
public:
    ParseResult parse(const std::string& csv_data) override {
        ParseResult result;
        auto start = std::chrono::steady_clock::now();
        
        try {
            // Парсер работает с файлами, поэтому создаем временный файл
            std::string tempFile = "/tmp/pedro_csv_temp_" + 
                                   std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + 
                                   ".csv";
            
            // Записываем данные во временный файл
            std::ofstream outFile(tempFile);
            if (!outFile.is_open()) {
                result.error = "Failed to create temp file";
                result.time_ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - start).count();
                return result;
            }
            outFile << csv_data;
            outFile.close();
            
            read_csv_t parser;  // класс из csv.hh
            
            // Открываем файл
            if (!parser.open(tempFile)) {
                result.error = "Failed to open CSV file";
                // Удаляем временный файл
                std::remove(tempFile.c_str());
                result.time_ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - start).count();
                return result;
            }
            
            // Читаем все строки
            std::vector<std::vector<std::string>> rows;
            std::vector<std::string> row;
            
            // Метод read_row() возвращает пустой вектор при достижении конца файла
            while (true) {
                row = parser.read_row();
                if (row.empty()) {
                    break;
                }
                rows.push_back(row);
            }
            
            result.data = rows;
            result.error = "";
            
            // Закрываем и удаляем временный файл
            std::remove(tempFile.c_str());
            
        } catch (const std::exception& e) {
            result.error = e.what();
        }
        
        auto end = std::chrono::steady_clock::now();
        result.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        return result;
    }
    
    std::string name() const override {
        return "PedroVicenteCSV (student project)";
    }
};

static auto _pedrovicente = (ParserManager::registerParser("pedrovicente", []() {
    return std::make_unique<PedroVicenteCSVParser>();
}), 0);