cmake_minimum_required(VERSION 3.17)
project(imc_stray)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Network)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)

file(GLOB_RECURSE IMC_SOURCES
        imc-cxx/IMC/Spec/Announce.hpp
        imc-cxx/IMC/Base/*.hpp
        imc-cxx/IMC/Base/*.cpp)

include_directories(${PROJECT_NAME} imc-cxx)

add_executable(${PROJECT_NAME}
        main.cpp
        Window.cpp
        Window.hpp
        systray.qrc
        AnnounceListener.hpp
        ${IMC_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -s -Wall -Wextra -pedantic-errors -flto -DNDEBUG")

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
target_link_libraries(${PROJECT_NAME} "pthread")

install(TARGETS ${PROJECT_NAME})
