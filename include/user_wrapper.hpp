#ifndef USER_WRAPPER_H
#define USER_WRAPPER_H

#include <memory>
#include <map>
#include <functional>
#include <string>
#include <chrono>
#include <fstream>

struct ParseResult {
    std::vector<std::vector<std::string>> data;
    std::string error;
    double time_ms = 0;
};

// Abstract class
class CSVParser {
	public:
        virtual ~CSVParser() = default;
		virtual ParseResult parse (const std::string& csv_data) = 0;
        virtual std::string name() const = 0;
};

// Singleton
class ParserManager {
    private:
        ParserManager() = default;
        static ParserManager& instance ();
        std::map<std::string, std::function<std::unique_ptr<CSVParser>()>> factories;
    public:
        static void registerParser(const std::string& key, std::function<std::unique_ptr<CSVParser>()> factory);
        static std::unique_ptr<CSVParser> getParser (const std::string& key);
};

#endif