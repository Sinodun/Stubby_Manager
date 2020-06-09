cmake_minimum_required(VERSION 3.5)

project(stubby_gui_proto LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if (WIN32 OR MINGW OR MSYS OR CYGWIN)
  set(HOSTOS "windows")
elseif (APPLE)
  set(HOSTOS "macos")
else ()
  message(FATAL_ERROR "Support limited to Windows, MacOS and Linux.")
endif ()
set(os_src_dir "os/${HOSTOS}")

# Windows.
if (WIN32)
  set(system_libs "")
  set(static_lib_suffix "_static")
  list(APPEND system_libs
    "ws2_32"
    "crypt32"
    "gdi32"
    "iphlpapi"
    "psapi"
    "userenv"
    "wlanapi"
  )
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
if (WIN32)
  find_package(Poco COMPONENTS Net REQUIRED)
elseif (APPLE)
  find_package(Qt5 COMPONENTS Macextras REQUIRED)
endif()

add_executable(stubby_gui_proto
  main.cpp
  mainwindow.cpp
  mainwindow.ui
  servicemanager.cpp 
  systemdnsmanager.cpp
  systray.qrc
  ${os_src_dir}/servicemanager_${HOSTOS}.cpp
  ${os_src_dir}/systemdnsmanager_${HOSTOS}.cpp
)

if(WIN32)
  target_sources(stubby_gui_proto PRIVATE ${os_src_dir}/networkinterface_${HOSTOS}.cpp)
elseif(APPLE)
  target_sources(stubby_gui_proto PRIVATE ${os_src_dir}/runtask_${HOSTOS}.cpp)
endif()

target_include_directories(stubby_gui_proto
  PUBLIC
  .
  ${os_src_dir}
)

if(WIN32)
  target_link_libraries(stubby_gui_proto PUBLIC
    ${system_libs}
    PRIVATE
    Qt5::Widgets
    Poco::Net)
elseif(APPLE)
  target_link_libraries(stubby_gui_proto PRIVATE
    "-framework Foundation"
    "-framework AppKit"
    "-framework Security"
    Qt5::Widgets
    Qt5::MacExtras)
endif()
