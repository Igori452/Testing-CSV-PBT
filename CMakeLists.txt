cmake_minimum_required(VERSION 3.15)
project(CSV_Parser_Test)

# =============================================
# НАСТРОЙКИ ПОЛЬЗОВАТЕЛЯ (ТОЛЬКО ЭТО МЕНЯТЬ!)
# =============================================

# Способ подключения: "CMAKE", "HEADER_ONLY", "SOURCE_FILES", "PREBUILT"
set(PARSER_TYPE "SOURCE_FILES")

# Путь к парсеру:
#   - для CMAKE: путь к директории с CMakeLists.txt
#   - для HEADER_ONLY: путь к директории с заголовками
#   - для SOURCE_FILES: путь к директории с .cpp/.cc файлами
#   - для PREBUILT: имя библиотеки
set(PARSER_PATH "external/csv-parser")

# Дополнительные флаги (если нужны)
set(PARSER_COMPILE_OPTIONS "-Wno-unused-parameter" "-Wno-sign-compare")

# =============================================
# АВТОМАТИЧЕСКАЯ СБОРКА (НЕ ТРОГАТЬ)
# =============================================

# Создаем интерфейсный таргет для парсера
add_library(parser_interface INTERFACE)

if (${PARSER_TYPE} STREQUAL "CMAKE")
    # Вариант 1: Готовая CMake библиотека
    message(STATUS "Подключение парсера через CMake: ${PARSER_PATH}")
    add_subdirectory(${PARSER_PATH})
    target_link_libraries(parser_interface INTERFACE rapidcsv)
    
elseif(${PARSER_TYPE} STREQUAL "HEADER_ONLY")
    # Вариант 2: Header-only библиотека
    message(STATUS "Подключение header-only парсера: ${PARSER_PATH}")
    target_include_directories(parser_interface INTERFACE ${PARSER_PATH})
    
elseif(${PARSER_TYPE} STREQUAL "SOURCE_FILES")
    # Вариант 3: Исходники, которые надо компилировать
    message(STATUS "Подключение парсера из исходников: ${PARSER_PATH}")
    
    # Находим все .cpp и .cc файлы в указанной директории
    file(GLOB PARSER_SOURCES 
        "${PARSER_PATH}/*.cpp"
        "${PARSER_PATH}/*.cc"
    )
    
    if(NOT PARSER_SOURCES)
        message(FATAL_ERROR "Не найдены .cpp или .cc файлы в ${PARSER_PATH}")
    endif()
    
    # Создаем статическую библиотеку
    add_library(user_parser STATIC ${PARSER_SOURCES})
    
    # Добавляем путь к заголовкам
    target_include_directories(user_parser PUBLIC ${PARSER_PATH})
    
    # Применяем флаги компиляции
    if(PARSER_COMPILE_OPTIONS)
        target_compile_options(user_parser PRIVATE ${PARSER_COMPILE_OPTIONS})
    endif()
    
    target_link_libraries(parser_interface INTERFACE user_parser)
    
elseif(${PARSER_TYPE} STREQUAL "PREBUILT")
    # Вариант 4: Уже скомпилированная библиотека
    message(STATUS "Подключение prebuilt библиотеки: ${PARSER_PATH}")
    target_link_libraries(parser_interface INTERFACE ${PARSER_PATH})
    
else()
    message(FATAL_ERROR "Неизвестный тип PARSER_TYPE: ${PARSER_TYPE}")
endif()

# Применяем доп. флаги если есть
if(PARSER_COMPILE_OPTIONS)
    target_compile_options(parser_interface INTERFACE ${PARSER_COMPILE_OPTIONS})
endif()

# Подключаем RapidCheck
add_subdirectory(lib/rapidcheck)

# Создание пользовательского адаптера
file(GLOB USER_WRAPPERS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/user_wrappers/*.cpp")

add_library(user_adapter OBJECT ${USER_WRAPPERS_SOURCES})

target_include_directories(user_adapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PARSER_PATH}  # Добавляем путь к заголовкам парсера
)

# Связываем адаптер с парсером
target_link_libraries(user_adapter PUBLIC
    parser_interface
)

# Сборка для конечного исполняемого файла
add_executable(csv_test_runner 
    main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/csv_tester.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/user_manager.cpp
)

target_include_directories(csv_test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PARSER_PATH}  # Добавляем путь к заголовкам парсера
)

target_sources(csv_test_runner PRIVATE
    $<TARGET_OBJECTS:user_adapter>
)

target_link_libraries(csv_test_runner PRIVATE
    rapidcheck
    parser_interface  # Линкуем интерфейс парсера
)