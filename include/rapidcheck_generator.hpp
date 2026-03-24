#ifndef RAPIDCHECK_GENERATOR_HPP
#define RAPIDCHECK_GENERATOR_HPP

#include "../lib/rapidcheck/include/rapidcheck.h"
#include <vector>
#include <string>

namespace rc {
namespace gen {

// Использовать gen::inRange с безопасными диапазонами
inline Gen<std::string> simpleString() {
    return gen::container<std::string>(
        gen::oneOf(
            gen::inRange<char>('a', 'z'),  // a-z
            gen::inRange<char>('A', 'Z'),  // A-Z
            gen::inRange<char>('0', '9')   // 0-9
        )
    );
}

// CSV поле
inline Gen<std::string> csvField() {
    return simpleString();
}

// CSV строка
inline Gen<std::vector<std::string>> csvRow() {
    return gen::container<std::vector<std::string>>(
        csvField()
    );
}

// CSV таблица
inline Gen<std::vector<std::vector<std::string>>> csvTable() {
    return gen::container<std::vector<std::vector<std::string>>>(
        csvRow()
    );
}

// CSV таблица с фиксированными размерами (просто обрезаем)
inline Gen<std::vector<std::vector<std::string>>> csvTable(int rows, int cols) {
    return gen::map(
        gen::container<std::vector<std::vector<std::string>>>(
            gen::container<std::vector<std::string>>(csvField())
        ),
        [rows, cols](std::vector<std::vector<std::string>> table) {
            // Обрезаем до rows строк
            if (table.size() > static_cast<size_t>(rows)) {
                table.resize(rows);
            }
            
            // Обрезаем каждую строку до cols колонок
            for (auto& row : table) {
                if (row.size() > static_cast<size_t>(cols)) {
                    row.resize(cols);
                }
            }
            
            return table;
        }
    );
}

// CSV → string
inline Gen<std::string> csvString() {
    return gen::map(csvTable(),
        [](const std::vector<std::vector<std::string>> &table) {
            std::string result;

            for (size_t i = 0; i < table.size(); i++) {
                for (size_t j = 0; j < table[i].size(); j++) {
                    result += table[i][j];
                    if (j + 1 < table[i].size())
                        result += ",";
                }
                if (i + 1 < table.size())
                    result += "\n";
            }
            return result;
        }
    );
}

// Поврежденный CSV
inline Gen<std::string> corruptedCSV() {
    return gen::apply(
        [](std::string csv, int pos, int type, char c) {
            if (csv.empty()) return csv;

            pos = pos % (csv.size() + 1);

            switch (type % 4) {
                case 0:
                    if (!csv.empty() && pos < csv.size())
                        csv.erase(pos, 1);
                    break;
                case 1:
                    csv.insert(pos, 1, c);
                    break;
                case 2:
                    if (pos < csv.size())
                        csv[pos] = '"';
                    break;
                case 3:
                    csv.insert(pos, "\n");
                    break;
            }
            return csv;
        },
        csvString(),
        gen::arbitrary<int>(),
        gen::arbitrary<int>(),
        gen::inRange<char>(32, 127)
    );
}

// Повреждение существующего CSV (без уровней силы)
inline Gen<std::string> corruptedCSV(const std::string& original) {
    return gen::apply(
        [original](int type, int pos, char c) {
            std::string csv = original;
            if (csv.empty()) return csv;
            
            pos = pos % (csv.size() + 1);
            
            switch (type % 4) {
                case 0: // удаление символа
                    if (!csv.empty() && pos < csv.size())
                        csv.erase(pos, 1);
                    break;
                case 1: // вставка символа
                    csv.insert(pos, 1, c);
                    break;
                case 2: // замена на кавычку
                    if (pos < csv.size())
                        csv[pos] = '"';
                    break;
                case 3: // вставка переноса строки
                    csv.insert(pos, "\n");
                    break;
            }
            return csv;
        },
        gen::arbitrary<int>(),
        gen::arbitrary<int>(),
        gen::inRange<char>(32, 127)
    );
}

// Boundary cases
inline Gen<std::string> boundaryCSV() {
    return gen::element(
        std::string(""),
        std::string("\n"),
        std::string(",\n"),
        std::string("\"\"\n"),
        std::string("a,\n"),
        std::string(",a\n"),
        std::string("a,b,c\n"),
        std::string("\"a\",\"b\",\"c\"\n"),
        std::string("\"a,b\",c\n")
    );
}

} // namespace gen
} // namespace rc

#endif