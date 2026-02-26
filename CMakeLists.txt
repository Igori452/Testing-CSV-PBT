cmake_minimum_required(VERSION 3.15)
project(CSV_Parser_Test)

# =============================================
# НАСТРОЙКИ ПОЛЬЗОВАТЕЛЯ (ТОЛЬКО ЭТО МЕНЯТЬ!)
# =============================================

# Способ подключения: "CMAKE", "HEADER_ONLY", "SOURCE_FILES", "PREBUILT"
set(PARSER_TYPE "HEADER_ONLY")

# Имя таргета (для CMAKE) или путь к заголовку (для HEADER_ONLY)
set(PARSER_TARGET_OR_PATH "include/myparser.hpp")

# Дополнительные флаги (если нужны)
set(PARSER_COMPILE_OPTIONS "")

# =============================================
# АВТОМАТИЧЕСКАЯ СБОРКА (НЕ ТРОГАТЬ)
# =============================================

# Создаем интерфейсный таргет для парсера
add_library(parser_interface INTERFACE)

if (${PARSER_TYPE} STREQUAL "CMAKE")
    # Вариант 1: Готовая CMake библиотека
    add_subdirectory(.)
    target_link_libraries(parser_interface INTERFACE ${PARSER_TARGET_OR_PATH})
    
elseif(${PARSER_TYPE} STREQUAL "HEADER_ONLY")
    # Вариант 2: Один или несколько заголовков
    get_filename_component(HEADER_DIR ${PARSER_TARGET_OR_PATH} DIRECTORY)
    target_include_directories(parser_interface INTERFACE ${HEADER_DIR})
    
elseif(${PARSER_TYPE} STREQUAL "SOURCE_FILES")
    # Вариант 3: Исходники, которые надо компилировать
    file(GLOB PARSER_SOURCES "src/*.cpp" "*.cpp")
    add_library(user_parser STATIC ${PARSER_SOURCES})
    target_include_directories(user_parser PUBLIC include)
    target_link_libraries(parser_interface INTERFACE user_parser)
    
elseif(${PARSER_TYPE} STREQUAL "PREBUILT")
    # Вариант 4: Уже скомпилированная библиотека
    target_link_libraries(parser_interface INTERFACE ${PARSER_TARGET_OR_PATH})
endif()

# Применяем доп. флаги если есть
if (PARSER_COMPILE_OPTIONS)
    target_compile_options(parser_interface INTERFACE ${PARSER_COMPILE_OPTIONS})
endif()

# =============================================
# АДАПТЕР ПОЛЬЗОВАТЕЛЯ
# =============================================
add_library(user_adapter STATIC parser_wrapper.cpp)
target_link_libraries(user_adapter PRIVATE parser_interface)
target_include_directories(user_adapter PRIVATE .)

# =============================================
# ТЕСТОВЫЙ ФРЕЙМВОРК
# =============================================
add_executable(csv_test_runner main.cpp)
target_link_libraries(csv_test_runner PRIVATE user_adapter)