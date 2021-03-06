# Версия CMake
cmake_minimum_required(VERSION 3.15)

# Название проекта (решение в Visual Studio)
project(BaseGraphics)

# Стандарт С/С++
set(CMAKE_CXX_STANDARD 14)

# Устанавливаем каталоги для бинарников
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

# Определить разрядность платформы
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(PLATFORM_BIT_SUFFIX "x86")
else()
    set(PLATFORM_BIT_SUFFIX "x64")
endif()

# Стандартные библиотеки для GNU/MinGW
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

# Библиотека для работы с математикой (header-only)
add_subdirectory("Sources/Math")

# Библиотека для работы с графикой (header-only)
add_subdirectory("Sources/Gfx")

# Библилтека вспомогательных инструментов
add_subdirectory("Sources/Tools")

# Примеры приложений
add_subdirectory("Sources/01_SamplePoint")
add_subdirectory("Sources/02_SampleLines")
add_subdirectory("Sources/03_SamplePointRotation")
add_subdirectory("Sources/04_SamplePointProjection")
add_subdirectory("Sources/05_SamplePolygonalDraw")
add_subdirectory("Sources/06_SampleSkeletalBasics")
add_subdirectory("Sources/07_RandomVectorWithinCone")
