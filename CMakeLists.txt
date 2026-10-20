cmake_minimum_required(VERSION 3.16)
project(obs-streamelements VERSION 26.9.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(OBS REQUIRED libobs obs-frontend-api)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineCore
    WebEngineWidgets
    WebSockets
    Network
)

find_package(CURL REQUIRED)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/plugin-main.cpp
    src/MenuManager.cpp
    src/BrowserDock.cpp
    src/BrowserWidgetManager.cpp
    src/ApiServer.cpp
    src/AudioWrapper.cpp
    src/BackupManager.cpp
    src/AuthManager.cpp
    src/AuthServer.cpp
    src/LoginDialog.cpp
)

set(HEADERS
    src/MenuManager.hpp
    src/BrowserDock.hpp
    src/BrowserWidgetManager.hpp
    src/ApiServer.hpp
    src/AudioWrapper.hpp
    src/BackupManager.hpp
    src/AuthManager.hpp
    src/AuthServer.hpp
    src/LoginDialog.hpp
)

add_library(obs-streamelements MODULE
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(obs-streamelements PRIVATE
    src
    ${OBS_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(obs-streamelements PRIVATE
    ${OBS_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::WebSockets
    Qt6::Network
    ${CURL_LIBRARIES}
)

set_target_properties(obs-streamelements PROPERTIES
    PREFIX ""
    OUTPUT_NAME "obs-streamelements"
)

# Install destination for user directory
set(INSTALL_LIB_DIR "$ENV{HOME}/.config/obs-studio/plugins/obs-streamelements/bin/64bit" CACHE PATH "Installation library directory")
set(INSTALL_DATA_DIR "$ENV{HOME}/.config/obs-studio/plugins/obs-streamelements/data" CACHE PATH "Installation data directory")

install(TARGETS obs-streamelements
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
)

install(DIRECTORY data/
    DESTINATION ${INSTALL_DATA_DIR}
)
