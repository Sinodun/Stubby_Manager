cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif ()

# The following must be set BEFORE doing project() or enable_language().
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type defined; defaulting to 'Debug'")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "The type of build. Possible values are: Debug, Release, RelWithDebInfo and MinSizeRel.")
endif ()

set(PACKAGE_NAME "Stubby Manager")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 4)
set(PACKAGE_VERSION_RELEASE 0)
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_RELEASE}")
set(PACKAGE_BUGREPORT "info@sinodun.com")
set(RELEASE_CANDIDATE "-a1")

set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}${RELEASE_CANDIDATE}")
set(PACKAGE_TARNAME "${PACKAGE}-${PACKAGE_VERSION}${RELEASE_CANDIDATE}")

if (WIN32)
  project (stubby_gui VERSION ${PACKAGE_VERSION} LANGUAGES CXX RC)
else()
  project (stubby_gui VERSION ${PACKAGE_VERSION} LANGUAGES CXX)
endif()

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
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(HOSTOS "linux")
else ()
  message(FATAL_ERROR "Support limited to Windows, MacOS and Linux.")
endif ()
set(os_src_dir "os/${HOSTOS}")

# Windows.
if (WIN32)
  option(PATCH_LEVEL_GIT "Get patch level from git." OFF)

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
find_package(yaml-cpp REQUIRED)
find_package(Libctemplate REQUIRED)
if (WIN32)
  find_package(Poco COMPONENTS Net REQUIRED)
  find_package(Qt5 COMPONENTS Network REQUIRED)

  set(PACKAGE_VERSION_PATCH 0)
  if (PATCH_LEVEL_GIT)
    execute_process(COMMAND git describe --long OUTPUT_VARIABLE ver)
    if (ver)
      string(REGEX REPLACE "v[^-]*-([0-9]+)-.*" "\\1" PACKAGE_VERSION_PATCH ${ver})
    endif ()
  endif ()

  set(VER_PRODUCTVERSION "${PACKAGE_VERSION_MAJOR},${PACKAGE_VERSION_MINOR},${PACKAGE_VERSION_RELEASE},${PACKAGE_VERSION_PATCH}")
  set(VER_PRODUCTVERSION_STR "${PACKAGE_VERSION}${RELEASE_CANDIDATE}")
  configure_file(${os_src_dir}/stubbygui.rc.in stubbygui.rc @ONLY)
  configure_file(${os_src_dir}/stubbygui.ico stubbygui.ico COPYONLY)
  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/stubby.qrc")
  set(WINDOWS_PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_RELEASE}.${PACKAGE_VERSION_PATCH}")
  configure_file(${os_src_dir}/windows_package_version.in windows_package_version @ONLY)
elseif (APPLE)
  find_package(Qt5 COMPONENTS Macextras REQUIRED)
endif()

if (WIN32)
    set(PLAFORM_BUILD WIN32)
elseif (APPLE)
     set(PLAFORM_BUILD MACOSX_BUNDLE)
else()
      set(PLAFORM_BUILD "")
endif()
add_executable( stubby_gui ${PLAFORM_BUILD}
  main.cpp
  mainwindow.cpp
  mainwindow.ui
  networkprofilewidget.cpp
  networkprofilewidget.ui
  networkprofiledelegate.cpp
  networkswidget.cpp
  networkswidgetfilterproxymodel.cpp
  networkswidget.ui
  config.cpp
  configmanager.cpp
  networkprofiletablemodel.cpp
  serverstablemodel.cpp
  servicemanager.cpp
  networkmanager.cpp
  stubby.qrc
  ${os_src_dir}/configmanager_${HOSTOS}.cpp
  ${os_src_dir}/servicemanager_${HOSTOS}.cpp
  ${os_src_dir}/networkmanager_${HOSTOS}.cpp
  ${os_src_dir}/logmanager_${HOSTOS}.cpp
  ${APP_ICON_RESOURCE_WINDOWS}
)

if(WIN32)
  target_sources(stubby_gui PRIVATE ${os_src_dir}/networkinterface_${HOSTOS}.cpp)
  SET(WINDOWS_RESOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/stubbygui.rc)
  target_sources(stubby_gui PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/stubbygui.rc)

  target_link_options(stubby_gui PRIVATE "/MANIFESTUAC:level='requireAdministrator'")
elseif(APPLE)
  target_sources(stubby_gui PRIVATE ${os_src_dir}/runtask_${HOSTOS}.cpp)
endif()

target_include_directories(stubby_gui
  PUBLIC
  .
  ${LIBCTEMPLATE_INCLUDE_DIR}
  ${YAML_CPP_INCLUDE_DIR}
  ${os_src_dir}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(WIN32)
  target_link_libraries(stubby_gui PUBLIC
    ${system_libs}
    PRIVATE
    Qt5::Network
    Qt5::Widgets
    Poco::Net
    Libctemplate::Libctemplate
    ${YAML_CPP_LIBRARIES})
elseif(APPLE)
  target_link_libraries(stubby_gui PRIVATE
    "-framework Foundation"
    "-framework AppKit"
    "-framework Security"
    Qt5::Widgets
    Qt5::MacExtras
    Libctemplate::Libctemplate
    ${YAML_CPP_LIBRARIES})
else()
  target_link_libraries(stubby_gui PRIVATE
    Qt5::Widgets
    Libctemplate::Libctemplate
    ${YAML_CPP_LIBRARIES})
endif()
