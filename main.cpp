#include "include/user_wrapper.h"
#include <iostream>

int main() {
    
    auto parsers = ParserManager::getParser("rapidcsv");

    parsers->parse("sdsdsd sd");

    return 0;
}