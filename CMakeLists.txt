cmake_minimum_required(VERSION 3.20)

project(
  uncrash
  VERSION 0.0.3
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Configure version header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(
  Qt6 REQUIRED
  COMPONENTS Core
             Gui
             Qml
             Quick
             Widgets
             DBus
             Network)

find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n Kirigami)

# ==============================================================================
# Daemon executable (uncrashd)
# ==============================================================================

add_executable(
  uncrashd
  src/daemon/main.cpp
  src/daemon/daemonservice.cpp
  src/daemon/daemonservice.h
  src/powermonitor.cpp
  src/powermonitor.h
  src/cpucontroller.cpp
  src/cpucontroller.h
  src/systemprotector.cpp
  src/systemprotector.h)

target_include_directories(uncrashd PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(uncrashd PRIVATE Qt6::Core Qt6::DBus Qt6::Network
                                       KF6::CoreAddons KF6::I18n)

install(TARGETS uncrashd ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# ==============================================================================
# GUI executable (uncrash)
# ==============================================================================

qt_add_executable(
  uncrash
  src/main.cpp
  src/utils/cli.cpp
  src/utils/cli.h
  src/client/daemonclient.cpp
  src/client/daemonclient.h
  src/settingsmanager.cpp
  src/settingsmanager.h)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(
  uncrash
  URI
  uncrash
  VERSION
  1.0
  OUTPUT_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/qml/uncrash
  NO_LINT
  NO_PLUGIN
  QML_FILES
  src/main.qml)

target_include_directories(uncrash PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
  uncrash
  PRIVATE Qt6::Core
          Qt6::Gui
          Qt6::Qml
          Qt6::Quick
          Qt6::Widgets
          Qt6::DBus
          Qt6::Network
          KF6::CoreAddons
          KF6::I18n
          KF6::Kirigami)

install(TARGETS uncrash ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Install desktop file
install(FILES uncrash.desktop DESTINATION ${KDE_INSTALL_APPDIR})

# Install application icon
install(FILES src/uncrash.svg
        DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
