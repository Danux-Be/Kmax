add_library(kmax_filter_sepia MODULE example_sepia.cpp)

target_link_libraries(kmax_filter_sepia PRIVATE
    kmaxapi
    KF6::CoreAddons
    KF6::I18n
)

set_target_properties(kmax_filter_sepia PROPERTIES
    AUTOMOC ON
    PREFIX ""
)

install(TARGETS kmax_filter_sepia
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kmax
)
