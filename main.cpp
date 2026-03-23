#include "include/user_wrapper.hpp"
#include <iostream>

int main() {
    
    auto parsers = ParserManager::getParser("rapidcsv");

    parsers->parse("sdsdsd sd");

    return 0;
}