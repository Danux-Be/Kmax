# ─── kmaxapi: shared library carrying the plugin interfaces ───────────────
# Both the host executable and every out-of-tree plugin link against this
# library, so that Q_OBJECT vtables for the interfaces live in exactly one
# place.

add_library(kmaxapi SHARED
    plugins/iplugin.h
    plugins/iplugin.cpp
    plugins/isource.h
    plugins/isource.cpp
    plugins/ifilter.h
    plugins/ifilter.cpp
    plugins/iupscaler.h
    plugins/iupscaler.cpp
    plugins/iencoder.h
    plugins/iencoder.cpp
)

set_target_properties(kmaxapi PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Api
    OUTPUT_NAME kmaxapi
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

target_include_directories(kmaxapi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kmax>
)

target_link_libraries(kmaxapi PUBLIC
    Qt6::Core
    Qt6::Gui
    KF6::CoreAddons
    KF6::I18n
)

install(TARGETS kmaxapi
    EXPORT KmaxTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
    plugins/iplugin.h
    plugins/isource.h
    plugins/ifilter.h
    plugins/iupscaler.h
    plugins/iencoder.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kmax/plugins
)

# ─── kmax: the main application executable ───────────────────────────────
add_executable(kmax)

target_sources(kmax PRIVATE
    main.cpp

    core/job.cpp
    core/jobqueue.cpp
    core/pipeline.cpp

    plugins/pluginmanager.cpp

    sources/filesource.cpp
    upscalers/realesrganupscaler.cpp
    encoders/ffmpegencoder.cpp
    filters/ffmpegfilter.cpp

    ui/mainwindow.cpp
    ui/settingsdialog.cpp
    ui/jobwidget.cpp
)

kconfig_add_kcfg_files(kmax GENERATE_MOC core/kmaxsettings.kcfgc)

target_include_directories(kmax PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(kmax PRIVATE
    kmaxapi
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::WidgetsAddons
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::KIOFileWidgets
    KF6::Crash
    KF6::DBusAddons
    KF6::IconThemes
)

if(KMAX_HAVE_LIBDVD)
    target_sources(kmax PRIVATE sources/dvdsource.cpp)
    target_link_libraries(kmax PRIVATE PkgConfig::LIBDVDREAD PkgConfig::LIBDVDNAV)
    target_compile_definitions(kmax PRIVATE KMAX_HAVE_LIBDVD=1)
endif()

set_target_properties(kmax PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS kmax ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES core/kmaxsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
