#include "../include/user_wrapper.h"

ParserManager& ParserManager::instance () {
    static ParserManager parserManager;
    return parserManager;
}

void ParserManager::registerParser(const std::string& key, std::function<std::unique_ptr<CSVParser>()> factory) {
    instance().factories[key] = factory;
}

std::unique_ptr<CSVParser> ParserManager::getParser (const std::string& key) {
    auto& factories = instance().factories;
    auto it = factories.find(key);
    
    if (it != factories.end()) return it->second(); // вызываем фабрику
    return nullptr;
}
