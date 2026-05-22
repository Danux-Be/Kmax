cmake_minimum_required(VERSION 3.20)

project(kmax
    VERSION 0.1.0
    DESCRIPTION "KDE video enhancement and upscaling application"
    HOMEPAGE_URL "https://github.com/Danux-Be/Kmax"
    LANGUAGES CXX
)

set(KMAX_VERSION_SUFFIX "" CACHE STRING "Version suffix, e.g. -alpha, -rc1")
set(KMAX_VERSION_STRING "${PROJECT_VERSION}${KMAX_VERSION_SUFFIX}")

set(QT_MIN_VERSION 6.7.0)
set(KF_MIN_VERSION 6.0.0)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(FeatureSummary)
include(GNUInstallDirs)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KMAX
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmax_version.h"
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
    Config
    ConfigWidgets
    WidgetsAddons
    KIO
    Crash
    DBusAddons
    IconThemes
)

# Optional: libdvdread / libdvdnav for DVD source
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBDVDREAD IMPORTED_TARGET dvdread)
    pkg_check_modules(LIBDVDNAV IMPORTED_TARGET dvdnav)
endif()

if(LIBDVDREAD_FOUND AND LIBDVDNAV_FOUND)
    set(KMAX_HAVE_LIBDVD TRUE)
    message(STATUS "Kmax: libdvdread/libdvdnav found — DVD source enabled")
else()
    set(KMAX_HAVE_LIBDVD FALSE)
    message(STATUS "Kmax: libdvdread/libdvdnav NOT found — DVD source disabled")
endif()

add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(plugins)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
